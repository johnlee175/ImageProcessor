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
package com.johnsoft.swing.actions;

import com.johnsoft.UiFace;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-01
 */
public class DelegateAction implements UiFace.Action {
    public interface Delegate {
        UiFace.Action getInstance();
    }

    private final Delegate delegate;

    public DelegateAction(Delegate delegate) {
        this.delegate = delegate;
    }

    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        delegate.getInstance().action(uiFace, control);
    }
}
