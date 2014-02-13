

package org.devicehive.glass;

import java.util.Hashtable;

import org.devicehive.glass.PhillipsHueDevice.NotificationsListener;

import com.dataart.android.devicehive.Command;
import com.dataart.android.devicehive.DeviceData;
import com.dataart.android.devicehive.Notification;
import com.dataart.android.devicehive.network.DeviceHiveResultReceiver;
import com.dataart.android.devicehive.network.NetworkCommandConfig;
import com.dataart.android.devicehive.network.DeviceHiveResultReceiver.ResultListener;
//import com.example.devicehive.android.client.sample.BuildConfig;
import com.dataart.devicehive.glass.BuildConfig;
import com.dataart.devicehive.glass.R;
import com.google.gson.Gson;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.KeyEvent;
import android.widget.Chronometer;
import android.widget.ImageView;
import android.widget.TextView;


public class NeuralyzerActivity extends Activity implements NotificationsListener, ResultListener {

	
	private TextView mHint;
	private ImageView imageView;

	private long mBase = 0;
	private boolean mStarted = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.layout_stopwatch);

		
		mHint = (TextView) findViewById(R.id.phrase_hint);

		imageView = (ImageView) findViewById(R.id.image);
		
		Light(false);

	}

	/**
	 * Handle the tap event from the touchpad.
	 */

	boolean isOn = false;

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		
		switch (keyCode) {
		// Handle tap events.

		case KeyEvent.KEYCODE_DPAD_CENTER:
		case KeyEvent.KEYCODE_ENTER:
			

			Light(isOn);
			isOn = !isOn;


			return true;
		default:
			return super.onKeyDown(keyCode, event);
		}
	}

	@Override
	public void onResume() {
		super.onResume();
		if (!mStarted) {
			
			
		}
	}

	@Override
	public void onPause() {
		super.onPause();
		if (mStarted) {
			
			mStarted = false;
		}
	}

	@Override
	public void onDestroy() {
		if (mStarted) {
			
			mStarted = false;
		}
		super.onDestroy();
	}

	

	PhillipsHueDevice client;
	Command command;

	DeviceHiveResultReceiver resultReceiver;

	public void Light(boolean isOn) {

		mHint.setText(isOn ? R.string.light_off : R.string.light_on);
		
		imageView.setImageResource(isOn ? R.drawable.light_on : R.drawable.light_off);
		

		if(resultReceiver == null) {
			resultReceiver = new DeviceHiveResultReceiver();
			resultReceiver.setResultListener(this, true);

			final NetworkCommandConfig config = new NetworkCommandConfig("http://nn2015.pg.devicehive.com/api",
					resultReceiver, BuildConfig.DEBUG);

			
		}


		if(client == null) {
			client = new PhillipsHueDevice(getApplicationContext());

			client.setApiEnpointUrl("http://nn2015.pg.devicehive.com/api");
			
			client.setDebugLoggingEnabled(BuildConfig.DEBUG);

			client.addNotificationsListener(this);
			client.startReceivingNotifications();
		}
		
		
		
		if (isOn) {
			client.lampOn();
		}
		else {
			client.lampOff();
		}
		
		
	}

	@Override
	public void onReceviceNotification(Notification notification) {
		// TODO Auto-generated method stub
		System.out.println( notification.toString());
	}

	@Override
	public void onReceiveResult(int code, int tag, Bundle data) {
		// TODO Auto-generated method stub

	}
}
