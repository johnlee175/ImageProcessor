#include "octree_palette.h"

__pack_begin struct _OctreeNode {
    bool			is_leaf;							/* true if node has no children */
    uint8_t	        color_index;						/* index of the palette */
    uint32_t	    pixel_count;						/* number of pixels represented by this leaf */
    uint32_t	    red_sum;							/* sum of red components */
    uint32_t	    green_sum;							/* sum of green components */
    uint32_t	    blue_sum;							/* sum of blue components */
    struct _OctreeNode*	children[8];					/* pointers to child nodes */
    struct _OctreeNode*	next;							/* pointer to next reducible node */
} __pack_end;
typedef struct _OctreeNode OctreeNode;

__pack_begin struct _ColorQuantization {
    uint32_t        max_pixel_count;
    size_t			leaf_count;
    OctreeNode*		reducible_nodes[8];
    OctreeNode*		tree_root;
} __pack_end;
/* typedef struct _ColorQuantization ColorQuantization; */ /* define ColorQuantization in header file */

_inline_ uint32_t calc_index_shift(uint32_t shift, uint8_t r, uint8_t g, uint8_t b, bool reverse) {
    if (reverse) {
        return ((r >> shift) & (uint32_t) 1) << 0 |
               ((g >> shift) & (uint32_t) 1) << 1 |
               ((b >> shift) & (uint32_t) 1) << 2;
    } else {
        return ((r >> shift) & (uint32_t) 1) << 2 |
               ((g >> shift) & (uint32_t) 1) << 1 |
               ((b >> shift) & (uint32_t) 1) << 0;
    }
}

static OctreeNode *create_node(ColorQuantization *quantization, size_t level) {
    DEFINE_HEAP_TYPE_POINTER(OctreeNode, node, {
        return NULL;
    });
    memset(node, 0, sizeof(OctreeNode));
    node->is_leaf = (level == 8) ? true : false;
    if (level == 8) {
        /* check the deepest level */
        ++quantization->leaf_count;
    } else {
        /* join all node horizontal */
        node->next = quantization->reducible_nodes[level];
        quantization->reducible_nodes[level] = node;
    }
    return node;
}

static bool add_color(ColorQuantization *quantization, uint8_t r, uint8_t g, uint8_t b) {
    OctreeNode *node = quantization->tree_root;
    if (node == NULL) return false;
    uint32_t index, shift, i;
    /* expand tree */
    for (i = 1; i <= 8; ++i) {
        if (node->is_leaf) break;
        shift = 8 - i;
        index = calc_index_shift(shift, r, g, b, false);
        if (node->children[index] == NULL) {
            node->children[index] = create_node(quantization, i);
            if (node->children[index] == NULL) {
                return false;
            }
        }
        node = node->children[index];
    }
    /* node is leaf */
    node->red_sum += r;
    node->green_sum += g;
    node->blue_sum += b;
    node->pixel_count++;
    return true;
}

static void reduce_color(ColorQuantization *quantization) {
    uint32_t red_sum = 0, green_sum = 0 , blue_sum = 0;
    uint32_t pixel_count = 0;
    size_t children = 0;
    int32_t idx;

    /* Find the deepest level containing at least one reducible node */
    for (idx = 7; idx > 0 && quantization->reducible_nodes[idx] == NULL; --idx);

    /* Reduce the node most recently added to the list at level idx */
    OctreeNode *node = quantization->reducible_nodes[idx];
    quantization->reducible_nodes[idx] = node->next;

    for (int32_t i = 0; i < 8; ++i) {
        if (node->children[i] != NULL) {
            red_sum	+= node->children[i]->red_sum;
            green_sum += node->children[i]->green_sum;
            blue_sum += node->children[i]->blue_sum;
            pixel_count += node->children[i]->pixel_count;
            free(node->children[i]);
            node->children[i] = NULL;
            children++;
        }
    }

    node->is_leaf = true;
    node->red_sum += red_sum;
    node->green_sum += green_sum;
    node->blue_sum += blue_sum;
    node->pixel_count += pixel_count;

    quantization->leaf_count -= (children - 1);
}

static void set_color_index(OctreeNode *node, int32_t *index) {
    if (node == NULL) return;
    if (node->is_leaf) {
        node->color_index = (uint8_t) *index;
        node->red_sum = node->red_sum / node->pixel_count;
        node->green_sum = node->green_sum / node->pixel_count;
        node->blue_sum = node->blue_sum / node->pixel_count;
        node->pixel_count = 1;
        ++*index;
    } else {
        for (int32_t i = 0; i < 8; ++i) {
            if (node->children[i] != NULL) {
                set_color_index(node->children[i], index);
            }
        }
    }
}

static void free_node(OctreeNode *tree) {
    if (tree == NULL) return;
    for(int32_t i = 0; i < 8; ++i) {
        if (tree->children[i] != NULL) {
            free_node(tree->children[i]);
        }
    }
    free(tree);
}

static void get_color_palette(OctreeNode *node, uint8_t *palette) {
    if (node->is_leaf) {
        int32_t base = node->color_index * 3;
        *(palette + base) = (uint8_t) node->red_sum;
        *(palette + base + 1) = (uint8_t) node->green_sum;
        *(palette + base + 2) = (uint8_t) node->blue_sum;
    } else {
        for (int32_t i = 0; i < 8; ++i) {
            if (node->children[i] != NULL) {
                get_color_palette(node->children[i], palette);
            }
        }
    }
}

bool generate_color_palette(ColorQuantization *quantization, uint8_t **palette/* user should free */) {
    if (quantization->tree_root != NULL) {
        ASSIGN_HEAP_ARRAY_POINTER(uint8_t, *palette, (quantization->max_pixel_count * 3), {
            return false;
        });
        get_color_palette(quantization->tree_root, *palette);
        return true;
    }
    return false;
}

ColorQuantization *create_color_quantization() {
    DEFINE_HEAP_TYPE_POINTER(ColorQuantization, quantization, {
        return NULL;
    });

    quantization->tree_root = create_node(quantization, 0);
    if (quantization->tree_root == NULL) {
        free(quantization);
        return NULL;
    }

    return quantization;
}

void destroy_color_quantization(ColorQuantization *quantization) {
    if (quantization == NULL) return;
    if (quantization->tree_root != NULL) {
        free_node(quantization->tree_root);
    }
    free(quantization);
}

bool prepare_color_palette(ColorQuantization *quantization, uint8_t *rgb_data,
                            uint32_t pixel_count, uint32_t max_colors, bool build_index) {
    if (max_colors > 256 || max_colors <= 0) {
        return false;
    }
    quantization->max_pixel_count = max_colors;
    uint8_t *end_ptr = rgb_data + pixel_count * 3;
    while (rgb_data < end_ptr) {
        if (!add_color(quantization, *rgb_data, *(rgb_data + 1), *(rgb_data + 2))) {
            return false;
        }
        while (quantization->leaf_count > max_colors) {
            reduce_color(quantization);
        }
        rgb_data += 3;
    }
    if (build_index) {
        int32_t index = 0;
        set_color_index(quantization->tree_root, &index);
    }
    return true;
}

int32_t index_of_palette(ColorQuantization *quantization, uint8_t r, uint8_t g, uint8_t b) {
    OctreeNode *node = quantization->tree_root;
    if (node == NULL) return -1;
    uint32_t index, shift, i;
    /* expand tree */
    for (i = 1; i <= 8; ++i) {
        if (node->is_leaf) break;
        shift = 8 - i;
        index = calc_index_shift(shift, r, g, b, false);
        if (node->children[index] == NULL) {
            return -1;
        }
        node = node->children[index];
    }
    return node->color_index;
}