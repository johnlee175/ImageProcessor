/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache license, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the license for the specific language governing permissions and
 * limitations under the license.
 */
package com.johnsoft.viewer;

import java.awt.AWTEvent;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.awt.Toolkit;
import java.awt.event.AWTEventListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Locale;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.filechooser.FileFilter;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-13
 */
public class GPcmPlayer {
    public static void main(String[] args) {
        Locale.setDefault(Locale.ENGLISH);
        EventQueue.invokeLater(new Runnable() {
            @Override
            public void run() {
                try {
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                } catch (Exception e) {
                    JOptionPane.showMessageDialog(null, "No optional system style UI skin");
                    e.printStackTrace();
                }
                showChooseFileDialog();
            }
        });
    }

    public static void showChooseFileDialog() {
        final PcmHeader header = new PcmHeader();
        final JDialog dialog = new JDialog((JFrame)null, true);
        final JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDragEnabled(true);
        fileChooser.setFileHidingEnabled(false);
        fileChooser.setMultiSelectionEnabled(false);
        fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
        fileChooser.setFileFilter(new FileFilter() {
            @Override
            public boolean accept(File f) {
                return f.isDirectory() || f.getName().endsWith(".pcm");
            }
            @Override
            public String getDescription() {
                return "*.pcm";
            }
        });
        fileChooser.setApproveButtonText("Open");
        fileChooser.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (JFileChooser.APPROVE_SELECTION.equals(e.getActionCommand())) {
                    header.file = fileChooser.getSelectedFile();
                }
                dialog.dispose();
            }
        });
        final JComboBox<Integer> sampleRateSelector = new JComboBox<>(new Integer[] {
                (int) PcmHeader.SAMPLE_RATE_8K, (int) PcmHeader.SAMPLE_RATE_11K, (int) PcmHeader.SAMPLE_RATE_16K,
                (int) PcmHeader.SAMPLE_RATE_22K, (int) PcmHeader.SAMPLE_RATE_32K, (int) PcmHeader.SAMPLE_RATE_44K,
                (int) PcmHeader.SAMPLE_RATE_48K
        });
        sampleRateSelector.setSelectedItem((int) PcmHeader.SAMPLE_RATE_16K);
        sampleRateSelector.setEditable(false);
        JLabel sampleRateLabel = new JLabel("sample rate: ");
        sampleRateLabel.setLabelFor(sampleRateSelector);
        final JComboBox<Integer> sampleSizeSelector = new JComboBox<>(new Integer[] {
                PcmHeader.SAMPLE_SIZE_8BIT_BYTE, PcmHeader.SAMPLE_SIZE_16BIT_SHORT,
                PcmHeader.SAMPLE_SIZE_32BIT_FLOAT
        });
        sampleSizeSelector.setSelectedItem(PcmHeader.SAMPLE_SIZE_16BIT_SHORT);
        sampleSizeSelector.setEditable(false);
        JLabel sampleSizeLabel = new JLabel("sample size: ");
        sampleSizeLabel.setLabelFor(sampleSizeSelector);
        final JComboBox<Integer> channelsSelector = new JComboBox<>(new Integer[] {
                PcmHeader.CHANNEL_MONO, PcmHeader.CHANNEL_STEREO
        });
        channelsSelector.setSelectedItem(PcmHeader.CHANNEL_MONO);
        channelsSelector.setEditable(false);
        JLabel channelsLabel = new JLabel("channels: ");
        channelsLabel.setLabelFor(channelsSelector);
        JCheckBox checkUnsigned = new JCheckBox("apply unsigned int");
        JCheckBox checkBigEndian = new JCheckBox("apply big endian");
        JPanel bottom = new JPanel(new FlowLayout());
        bottom.add(sampleRateLabel);
        bottom.add(sampleRateSelector);
        bottom.add(sampleSizeLabel);
        bottom.add(sampleSizeSelector);
        bottom.add(channelsLabel);
        bottom.add(channelsSelector);
        bottom.add(checkUnsigned);
        bottom.add(checkBigEndian);
        JPanel panel = new JPanel(new BorderLayout());
        panel.add(new JLabel("  which pcm file would you want to open: "), BorderLayout.NORTH);
        panel.add(fileChooser, BorderLayout.CENTER);
        panel.add(bottom, BorderLayout.SOUTH);
        dialog.setContentPane(panel);
        dialog.setTitle("Open File Dialog");
        dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        dialog.setAlwaysOnTop(true);
        Dimension preferredSize = panel.getPreferredSize();
        dialog.setSize(preferredSize);
        dialog.setMinimumSize(preferredSize);
        dialog.setLocationRelativeTo(null);
        dialog.setVisible(true);
        header.sampleRate = (int) sampleRateSelector.getSelectedItem();
        header.sampleSizeInBits = (int) sampleSizeSelector.getSelectedItem();
        header.sampleSizeInBytes = header.sampleSizeInBits / 8;
        header.channels = (int) channelsSelector.getSelectedItem();
        header.signed = !checkUnsigned.isSelected();
        header.bigEndian = checkBigEndian.isSelected();
        header.frameBufferSize = (int) (header.sampleRate * header.sampleSizeInBytes * header.channels);
        if (header.file.length() < 30 * 60 * header.frameBufferSize /* 30min */) {
            // 1s audio data
            header.frameBufferSize = header.frameBufferSize / 10; // 0.1s audio data
        }
        System.out.println(header);
        new GPcmPlayer(header);
    }

    private PcmHeader header;
    private byte[] frameBuffer;
    private FileChannel fileChannel;
    private SourceDataLine sdl;

    private volatile long position;
    private MessageThread thread;
    private MessageThread.Handler handler;

    private JFrame window;
    private JProgressBar seekBar;
    private JLabel seekLabel;
    private JToggleButton btnPauseResume;

    public GPcmPlayer(PcmHeader header) {
        this.header = header;
        initUi(header);
        try {
            initData(header);
        } catch (Exception e) {
            showError(e);
        }
        seekTo(0);
        resume();
    }

    private void initUi(PcmHeader header) {
        window = new JFrame("PcmPlayer[" + header.file.getName() + "]");
        final int[] mums = calcMums(header.frameBufferSize, header.file.length());
        seekBar = new JProgressBar(mums[0], mums[1]);
        seekBar.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getButton() == MouseEvent.BUTTON1 && e.getClickCount() == 1) {
                    final int maximum = seekBar.getMaximum();
                    final int minimum = seekBar.getMinimum();
                    int value = (int) ((e.getX() / (float) seekBar.getWidth()) * (maximum - minimum));
                    setValue(value, minimum, maximum);
                    seekTo(fromMum(value, minimum, maximum));
                }
            }
        });
        Toolkit.getDefaultToolkit().addAWTEventListener(new AWTEventListener() {
            @Override
            public void eventDispatched(AWTEvent event) {
                Object source = event.getSource();
                if (source == seekBar && event instanceof KeyEvent) {
                    final KeyEvent e = (KeyEvent) event;
                    final int keyCode = e.getKeyCode();
                    if (keyCode == KeyEvent.VK_LEFT) {
                        int value = seekBar.getValue();
                        final int minimum = seekBar.getMinimum();
                        final int maximum = seekBar.getMaximum();
                        if (value > minimum) {
                            setValue(--value, minimum, maximum);
                            seekTo(fromMum(value, minimum, maximum));
                        }
                    } else if (keyCode == KeyEvent.VK_RIGHT) {
                        int value = seekBar.getValue();
                        final int minimum = seekBar.getMinimum();
                        final int maximum = seekBar.getMaximum();
                        if (value < maximum) {
                            setValue(++value, minimum, maximum);
                            seekTo(fromMum(value, minimum, maximum));
                        }
                    }
                }
            }
        }, AWTEvent.KEY_EVENT_MASK);
        seekBar.setFocusable(true);
        seekBar.setPreferredSize(new Dimension(600, 30));
        seekLabel = new JLabel(seekBar.getMinimum() + "/(" + seekBar.getMinimum() + "-" + seekBar.getMaximum() + ")");
        seekLabel.setHorizontalAlignment(SwingConstants.RIGHT);
        seekLabel.setPreferredSize(new Dimension(150, 30));
        btnPauseResume = new JToggleButton("pause");
        btnPauseResume.setPreferredSize(new Dimension(90, 30));
        btnPauseResume.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (btnPauseResume.isSelected()) {
                    btnPauseResume.setText("resume");
                    pause();
                } else {
                    btnPauseResume.setText("pause ");
                    resume();
                }
            }
        });
        final JPanel east = new JPanel();
        east.add(btnPauseResume);
        east.add(seekLabel);
        final JPanel content = new JPanel(new BorderLayout());
        content.add(east, BorderLayout.EAST);
        content.add(seekBar);
        window.setContentPane(content);
        window.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        window.pack();
        window.setLocationRelativeTo(null);
        window.setVisible(true);
    }

    private void initData(final PcmHeader header) throws LineUnavailableException, IOException  {
        if (header.sampleSizeInBits % 8 != 0) {
            throw new IOException("not support sample size which can't mod with 8");
        }
        frameBuffer = new byte[header.frameBufferSize];
        fileChannel = new FileInputStream(header.file).getChannel();

        final AudioFormat audioFormat = new AudioFormat((header.signed
                ? AudioFormat.Encoding.PCM_SIGNED : AudioFormat.Encoding.PCM_UNSIGNED),
                header.sampleRate,
                header.sampleSizeInBits,
                header.channels,
                header.sampleSizeInBytes * header.channels,
                header.sampleRate,
                header.bigEndian);
        final SourceDataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat, header.frameBufferSize);
        sdl = (SourceDataLine) AudioSystem.getLine(info);
        sdl.open(audioFormat);

        thread = new MessageThread();
        handler = new MessageThread.Handler() {
            private boolean doing;
            @Override
            void handleMessage(MessageThread.Message message) {
                if ("resume".equals(message.what)) {
                    sdl.start();
                    doing = true;
                    sendMessage(new MessageThread.Message("playFrame"));
                } else if ("pause".equals(message.what)) {
                    doing = false;
                    sdl.stop();
                } else if ("playFrame".equals(message.what)) {
                    if (doing) {
                        try {
                            long nextPosition = playFrame() + position();
                            seekTo(nextPosition);
                            final int minimum = seekBar.getMinimum();
                            final int maximum = seekBar.getMaximum();
                            setValue(toMum(nextPosition, minimum, maximum), minimum, maximum);
                            if (nextPosition < header.file.length()) {
                                sendMessage(new MessageThread.Message("playFrame"));
                            } else {
                                SwingUtilities.invokeLater(new Runnable() {
                                    @Override
                                    public void run() {
                                        btnPauseResume.setSelected(true);
                                        setValue(toMum(0, minimum, maximum), minimum, maximum);
                                    }
                                });
                                pause();
                                seekTo(0);
                            }
                        } catch (final Exception e) {
                            SwingUtilities.invokeLater(new Runnable() {
                                @Override
                                public void run() {
                                    showError(e);
                                }
                            });
                        }
                    }
                }
            }
        };
        thread.addHandler(handler);
        thread.start();
    }

    private void setValue(final int value, final int min, final int max) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                seekBar.setValue(value);
                seekLabel.setText(value + "/(" + min + "-" + max + ")");
                System.out.println("setValue: " + value);
            }
        });
    }

    private int playFrame() throws IOException {
        final int readCount = fileChannel.read(ByteBuffer.wrap(frameBuffer), position);
        int writeCount = 0;
        while ((writeCount += sdl.write(frameBuffer, 0, readCount)) < readCount);
        return writeCount;
    }

    private int[] calcMums(int frameBufferSize, long fileLength) {
        final int max = (int) (fileLength / frameBufferSize + (fileLength % frameBufferSize == 0 ? 0 : 1));
        return new int[] { 0, max };
    }

    private long fromMum(int value, int min, int max) {
        return value * header.frameBufferSize;
    }

    private int toMum(long position, int min, int max) {
        return (int) (((double) position / header.file.length()) * (max - min));
    }

    public long position() {
        return position;
    }

    public void seekTo(long position) {
        System.out.println("seekTo: " + position);
        this.position = position;
    }

    public void pause() {
        System.out.println("pause");
        handler.sendMessage(new MessageThread.Message("pause"));
    }

    public void resume() {
        System.out.println("resume");
        handler.sendMessage(new MessageThread.Message("resume"));
    }

    private static void showError(Exception e) {
        JOptionPane.showMessageDialog(null, e.getMessage(), "Catch Exception", JOptionPane.ERROR_MESSAGE);
        System.exit(-1);
    }

    public static final class PcmHeader {
        public static final float SAMPLE_RATE_8K = 8000F;
        public static final float SAMPLE_RATE_11K = 11025F;
        public static final float SAMPLE_RATE_16K = 16000F;
        public static final float SAMPLE_RATE_22K = 22050F;
        public static final float SAMPLE_RATE_32K = 32000F;
        public static final float SAMPLE_RATE_44K = 44100F;
        public static final float SAMPLE_RATE_48K = 48000F;

        public static final int SAMPLE_SIZE_8BIT_BYTE = 8;
        public static final int SAMPLE_SIZE_16BIT_SHORT = 16;
        public static final int SAMPLE_SIZE_32BIT_FLOAT = 32;

        public static final int CHANNEL_MONO = 1;
        public static final int CHANNEL_STEREO = 2;

        public File file;
        public float sampleRate;
        public int sampleSizeInBits;
        public int sampleSizeInBytes;
        public int channels;
        public boolean signed;
        public boolean bigEndian;
        public int frameBufferSize;

        @Override
        public String toString() {
            return "PcmHeader{" +
                    "file=" + file +
                    ", sampleRate=" + sampleRate +
                    ", sampleSizeInBits=" + sampleSizeInBits +
                    ", sampleSizeInBytes=" + sampleSizeInBytes +
                    ", channels=" + channels +
                    ", signed=" + signed +
                    ", bigEndian=" + bigEndian +
                    ", frameBufferSize=" + frameBufferSize +
                    '}';
        }
    }
}
