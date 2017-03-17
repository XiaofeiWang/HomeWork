package com.example.activitytasktest;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class SingleInstanceActvity extends Activity{

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_singleinstance);
		Button button = (Button)findViewById(R.id.show_actvity);
		button.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent intent = new Intent();
				ComponentName component = new ComponentName(SingleInstanceActvity.this, NormalActivity.class);
				intent.setComponent(component);
				startActivity(intent);
			}
		});
	}
}
