#include "glbox.h"

GLBox *glbox_create() {
    GLBox *glBox = (GLBox *) malloc(sizeof(GLBox));
    glBox->vsSource = NULL;
    glBox->fsSource = NULL;
    glBox->window = NULL;
    return glBox;
}

int glbox_load_shader_file(GLBox *box, const char *vs_file, const char *fs_file) {
    FILE *vsFp = fopen(vs_file, "r");
    fseek(vsFp, 0L, SEEK_END);
    long vs_len = ftell(vsFp);
    GLchar *vsSource = malloc((size_t) (vs_len + 1));
    memset(vsSource, 0, vs_len + 1);
    fseek(vsFp, 0L, SEEK_SET);
    fread(vsSource, (size_t) vs_len, 1, vsFp);
    fclose(vsFp);

    FILE *fsFp = fopen(fs_file, "r");
    fseek(fsFp, 0L, SEEK_END);
    long fs_len = ftell(fsFp);
    GLchar *fsSource = malloc((size_t) (fs_len + 1));
    memset(fsSource, 0, fs_len + 1);
    fseek(fsFp, 0L, SEEK_SET);
    fread(fsSource, (size_t) fs_len, 1, fsFp);
    fclose(fsFp);

    box->vsSource = vsSource;
    box->fsSource = fsSource;

    return 0;
}

int glbox_glfw_init_window_context(GLBox *box, int width, int height, const char *title) {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit()) return -1;

    box->window = window;
    box->window_width = width;
    box->window_height = height;
    return 0;
}

int glbox_viewport_with_window(GLBox *box) {
    glfwGetFramebufferSize(box->window, &box->viewport_width, &box->viewport_height);
    glViewport(0, 0, box->viewport_width, box->viewport_height);
    return 0;
}

int glbox_compile_link_shader(GLBox *box) {
    GLint success;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &box->vsSource, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vs, sizeof(infoLog), NULL, infoLog);
        glbox_error_log("ERROR::SHADER::VERTEX::COMPILATION_FAILED -> %s\n", infoLog);
        return -1;
    }

    GLuint  fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &box->fsSource, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fs, sizeof(infoLog), NULL, infoLog);
        glbox_error_log("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED -> %s\n", infoLog);
        return -1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        glbox_error_log("ERROR::SHADER::PROGRAM::LINKING_FAILED -> %s\n", infoLog);
        return -1;
    }

#ifdef DEBUG
    glValidateProgram(program); // just for develop debug
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        glbox_error_log("ERROR::SHADER::PROGRAM::VALIDATE_FAILED -> %s\n", infoLog);
        return -1;
    }
#endif

    glDeleteShader(vs);
    glDeleteShader(fs);
    box->program = program;
    return 0;
}

int glbox_loop(GLBox *box, setup_func setup, display_func display, teardown_func teardown) {
    int setup_success;
    if (setup != NULL && (setup_success = setup(box)) != 0) return setup_success;
    while (!glfwWindowShouldClose(box->window)) {
        glUseProgram(box->program);
        display(box);
        glfwSwapBuffers(box->window);
        glfwPollEvents();
    }
    if (teardown != NULL) {
        teardown(box);
    }
    glfwTerminate();
    return 0;
}

void glbox_destroy(GLBox *box) {
    free((void *) box->vsSource);
    free((void *) box->fsSource);
    glfwDestroyWindow(box->window);
    free(box);
}

void glbox_util_vertex_array_f(GLuint *vao, GLuint *vbo, GLuint program,
                               GLfloat *vertices, size_t vertices_size,
                               const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glGenVertexArrays(1, &*vao);
    glGenBuffers(1, &*vbo);

    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

    GLuint loc_attr_vertex = (GLuint) glGetAttribLocation(program, vertex_attrib_name);
    glVertexAttribPointer(loc_attr_vertex, 3, GL_FLOAT, GL_FALSE,
                          line_size * sizeof(GLfloat), (GLvoid*) (offset * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(loc_attr_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_util_use_vertex_array_f(GLuint vao, GLuint vbo, GLuint program,
                               const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLuint loc_attr_vertex = (GLuint) glGetAttribLocation(program, vertex_attrib_name);
    glVertexAttribPointer(loc_attr_vertex, 3, GL_FLOAT, GL_FALSE,
                          line_size * sizeof(GLfloat), (GLvoid*) (offset * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(loc_attr_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_util_vertex_element_f(GLuint *vao, GLuint *vbo, GLuint *ebo, GLuint program,
                                 GLfloat *vertices, size_t vertices_size, GLuint *indices, size_t indices_size,
                                 const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glGenVertexArrays(1, &*vao);
    glGenBuffers(1, &*vbo);
    glGenBuffers(1, &*ebo);

    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);

    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

    GLuint loc_attr_vertex = (GLuint) glGetAttribLocation(program, vertex_attrib_name);
    glVertexAttribPointer(loc_attr_vertex, 3, GL_FLOAT, GL_FALSE,
                          line_size * sizeof(GLfloat), (GLvoid*) (offset * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(loc_attr_vertex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_util_use_vertex_element_f(GLuint vao, GLuint vbo, GLuint ebo, GLuint program,
                                     const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    GLuint loc_attr_vertex = (GLuint) glGetAttribLocation(program, vertex_attrib_name);
    glVertexAttribPointer(loc_attr_vertex, 3, GL_FLOAT, GL_FALSE,
                          line_size * sizeof(GLfloat), (GLvoid*) (offset * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(loc_attr_vertex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_util_draw_vertex_array(GLuint vao, GLuint vbo, GLsizei triangle_count) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_util_draw_vertex_element(GLuint vao, GLuint vbo, GLuint ebo, GLsizei triangle_count) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glbox_vertex_array_f(GLBox *box, GLfloat *vertices, size_t vertices_size,
                          const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glbox_util_vertex_array_f(&box->vao, &box->vbo, box->program, vertices, vertices_size,
                              vertex_attrib_name, line_size, offset);
}

void glbox_use_vertex_array_f(GLBox *box, const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glbox_util_use_vertex_array_f(box->vao, box->vbo, box->program, vertex_attrib_name, line_size, offset);
}

void glbox_vertex_element_f(GLBox *box, GLfloat *vertices, size_t vertices_size,
                            GLuint *indices, size_t indices_size,
                            const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glbox_util_vertex_element_f(&box->vao, &box->vbo, &box->ebo, box->program, vertices, vertices_size,
                                indices, indices_size, vertex_attrib_name, line_size, offset);
}

void glbox_use_vertex_element_f(GLBox *box, const GLchar *vertex_attrib_name, GLint line_size, GLint offset) {
    glbox_util_use_vertex_element_f(box->vao, box->vbo, box->ebo, box->program,
                                    vertex_attrib_name, line_size, offset);
}

void glbox_draw_vertex_array(GLBox *box, GLsizei triangle_count) {
    glbox_util_draw_vertex_array(box->vao, box->vbo, triangle_count);
}

void glbox_draw_vertex_element(GLBox *box, GLsizei triangle_count) {
    glbox_util_draw_vertex_element(box->vao, box->vbo, box->ebo, triangle_count);
}

void glbox_util_uniform_1f(GLuint program, const GLchar *vertex_attrib_name, GLfloat value) {
    GLuint loc_uniform = (GLuint) glGetUniformLocation(program, vertex_attrib_name);
    glUniform1f(loc_uniform, value);
}

void glbox_util_uniform_Matrix4fv(GLuint program, const GLchar *vertex_attrib_name, const GLfloat *value) {
    GLuint loc_uniform = (GLuint) glGetUniformLocation(program, vertex_attrib_name);
    glUniformMatrix4fv(loc_uniform, 1, GL_TRUE, value);
}

void glbox_util_attribute_3f(GLuint program, const GLchar *vertex_attrib_name,
                             GLfloat v1, GLfloat v2, GLfloat v3) {
    GLuint loc_attr_vertex = (GLuint) glGetAttribLocation(program, vertex_attrib_name);
    glVertexAttrib3f(loc_attr_vertex, v1, v2, v3);
}

void glbox_util_draw_pixels(const void *pixels, GLsizei width, GLsizei height) {
    glRasterPos2i(-1, 1);
    glPixelZoom(1, -1);
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void glbox_util_save_pixels_ppm3(const char *ppm_file_path, GLsizei width, GLsizei height) {
    GLubyte *pixels = malloc((size_t) (width * height * 3));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    size_t i, j, k;
    FILE *f = fopen(ppm_file_path, "w");
    fprintf(f, "P3\n");
    fprintf(f, "%d %d\n", width, height);
    fprintf(f, "255\n");
    k = 0;
    for(i = 0; i < width; ++i) {
        for(j = 0; j < height; ++j) {
            fprintf(f, "%u %u %u ",
                    pixels[k],
                    pixels[k + 1],
                    pixels[k + 2]);
            k = k + 3;
        }
        fprintf(f, "\n");
    }
    fclose(f);
    free(pixels);
}

void glbox_save_pixels_ppm3(GLBox *box, const char *ppm_file_path) {
    glbox_util_save_pixels_ppm3(ppm_file_path, box->viewport_width, box->viewport_height);
}

int glbox_display_by_freeglut(int argc, char **argv, int width, int height, const char *title,
                              void (*display)(void)) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}

void glbox_prepare_display(GLclampf r, GLclampf g, GLclampf b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void glbox_util_texture_2d(GLuint *texture, int texture_index,
                           GLuint program, const GLchar *vertex_uniform_name, GLint st_param,
                           const void *pixels, GLsizei img_width, GLsizei img_height) {
    glGenTextures(1, texture);

    switch (texture_index) {
        case 1:
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 5);
            break;
        case 6:
            glActiveTexture(GL_TEXTURE6);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 6);
            break;
        default:
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 0);
            break;
    }

    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, st_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, st_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void glbox_util_use_texture_2d(GLuint texture, int texture_index,
                           GLuint program, const GLchar *vertex_uniform_name,
                           const void *pixels, GLsizei img_width, GLsizei img_height) {
    switch (texture_index) {
        case 1:
            glActiveTexture(GL_TEXTURE1);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 5);
            break;
        case 6:
            glActiveTexture(GL_TEXTURE6);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 6);
            break;
        default:
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(program, vertex_uniform_name), 0);
            break;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img_width, img_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void glbox_texture_2d(GLBox *box, int texture_index,const GLchar *vertex_uniform_name, GLint st_param,
                           const void *pixels, GLsizei img_width, GLsizei img_height) {
    glbox_util_texture_2d(&box->texture, box->texture_index = texture_index, box->program, vertex_uniform_name,
                          st_param, pixels, box->texture_width = img_width, box->texture_height = img_height);
}

void glbox_use_texture_2d(GLBox *box, const GLchar *vertex_uniform_name, const void *pixels) {
    glbox_util_use_texture_2d(box->texture, box->texture_index, box->program, vertex_uniform_name, pixels,
                              box->texture_width, box->texture_height);
}

int glbox_cocoa_context() {
    CGLContextObj context;
    CGLPixelFormatAttribute attributes[13] = {
            kCGLPFAOpenGLProfile,
            (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
            kCGLPFAAccelerated,
            kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
            kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
            kCGLPFADoubleBuffer,
            kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1,
            kCGLPFASamples,  (CGLPixelFormatAttribute)4,
            (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pix;
    CGLError errorCode;
    GLint num; // stores the number of possible pixel formats
    errorCode = CGLChoosePixelFormat(attributes, &pix, &num);
    if (errorCode != 0) {
        return -1;
    }
    errorCode = CGLCreateContext(pix, NULL, &context);
    if (errorCode != 0) {
        return -1;
    }
    CGLDestroyPixelFormat(pix);
    errorCode = CGLSetCurrentContext(context);
    if (errorCode != 0) {
        return -1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit()) return -1;
    return 0;
}

void glbox_viewport_with_fix_size(GLBox *box, int width, int height) {
    box->viewport_width = box->window_width = width;
    box->viewport_height = box->window_height = height;
    glViewport(0, 0, width, height);
}

int glbox_util_framebuffer(GLuint *framebuffer, GLsizei width, GLsizei height,
                           GLuint texture, GLenum attachment) {
    GLuint depthBufferName;
    glGenRenderbuffers(1, &depthBufferName);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

int glbox_framebuffer(GLBox *box, GLenum attachment) {
    return glbox_util_framebuffer(&box->fbo, box->window_width, box->window_height,
                                  box->texture, attachment);
}