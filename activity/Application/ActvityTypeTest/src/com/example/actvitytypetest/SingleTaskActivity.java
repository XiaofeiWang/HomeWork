package com.example.actvitytypetest;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class SingleTaskActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.singletask);
        Button launch = (Button)findViewById(R.id.launch_noraml);
        launch.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent intent = new Intent(SingleTaskActivity.this, MainActivity.class);
				SingleTaskActivity.this.startActivity(intent);
			}
		});
    }
}
