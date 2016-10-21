package com.example.windowtest;


import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class WindowMainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_window_main);
		Button showDialogButton = (Button)findViewById(R.id.show_dialog);
		showDialogButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				TextView textView = new TextView(WindowMainActivity.this);
				textView.setText("SUB window context");
				AlertDialog.Builder builder  = new AlertDialog.Builder(WindowMainActivity.this);
				builder.setTitle("SUB window");
				builder.setView(textView);
				builder.create().show();
			}
		});
	}
}
