package android.wangxiaofei;

import android.app.Activity;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.os.ServiceManager;

public class MainActivity extends Activity {
	public static final String TAG = "AccountService";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		IBinder b = ServiceManager.getService("accountservice");
        try {
            final IAccountService accountService = IAccountService.Stub.asInterface(b);
            Log.i(TAG, "accountService.getlocked(): " + accountService.getlocked());
            Log.i(TAG, "accountService.setlocked(): " + accountService.setlocked(1));
        } catch (Exception e) {
            Log.i(TAG, "Exception: " + e);
        }
	}
}
