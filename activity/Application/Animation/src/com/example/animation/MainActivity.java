
package com.example.animation;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.ViewFlipper;

import com.example.animation.R;

public class MainActivity extends Activity implements
AdapterView.OnItemSelectedListener {


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mFlipper = ((ViewFlipper) this.findViewById(R.id.flipper));
        mFlipper.startFlipping();

        Spinner s = (Spinner) findViewById(R.id.spinner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, mStrings);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s.setAdapter(adapter);
        s.setOnItemSelectedListener(this);
    }

    public void onItemSelected(AdapterView<?> parent, View v, int position, long id) {
            mFlipper.setInAnimation(AnimationUtils.loadAnimation(this,
                    R.anim.push_up_in));
            mFlipper.setOutAnimation(AnimationUtils.loadAnimation(this,
                    R.anim.push_up_out));
    }

    public void onNothingSelected(AdapterView<?> parent) {
    }

    private String[] mStrings = {
            "Push up", "Push left", "Cross fade", "Hyperspace"};

    private ViewFlipper mFlipper;
}
