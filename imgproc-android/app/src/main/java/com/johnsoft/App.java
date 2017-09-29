package com.johnsoft;

import android.app.Application;

/**
 * App
 * @author John Kenrinus Lee
 * @version 2017-09-28
 */
public class App extends Application {
    private static App app;

    public static App self() {
        return app;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        app = this;
    }
}
