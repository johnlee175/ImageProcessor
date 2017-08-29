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
package com.johnsoft.viewer.benchmark;

import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Fork;
import org.openjdk.jmh.annotations.Measurement;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.Threads;
import org.openjdk.jmh.annotations.Warmup;

import com.johnsoft.viewer.YCbCrFileParser;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-07
 */
@Fork(1)
@Warmup(iterations = 4, time = 4)
@Measurement(iterations = 4, time = 4)
@State(Scope.Benchmark)
@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
public class YuvTest {
    @Benchmark
    @Threads(1)
    public void calcYuvX() {
        YCbCrFileParser.aycbcr2argbX(255, 100, 22, 22);
    }

    @Benchmark
    @Threads(1)
    public void calcYuvXX() {
        YCbCrFileParser.aycbcr2argbXX(255, 100, 22, 22);
    }

    @Benchmark
    @Threads(1)
    public void calcYuvXXX() {
        YCbCrFileParser.aycbcr2argbXXX(255, 100, 22, 22);
    }
}
