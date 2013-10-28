package de.pleumann.railuinoapp;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Set;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;
import android.widget.TableRow;
import android.widget.Toast;
import android.widget.ToggleButton;

public class MainActivity extends Activity {

    public static final int DIALOG_LOCO = 1;
    
    public static final int DIALOG_ACCY = 2;
    
    public static final int DIALOG_LOCO_DELETE = 3;
    
    public static final int DIALOG_ACCY_DELETE = 4;
    
    private TableRow[] tableRows = new TableRow[10];
    
    private Spinner locoSpinner;
    
    private ImageButton locoConfigButton;

    private ImageView locoImageView;
    
    private SeekBar locoSpeedBar;
    
    private ImageButton locoSlowerButton;
    
    private ImageButton locoDirectButton;

    private ImageButton locoFasterButton;
    
    private ToggleButton[] locoFunctionButtons = new ToggleButton[10];
    
    private Spinner accySpinner;

    private ImageButton accyConfigButton;
    
    private ImageButton[] accyButtons = new ImageButton[10];

    private ArrayAdapter<Locomotive> locoAdapter;
    
    private ArrayAdapter<Accessory> accyAdapter;

    private ArrayAdapter<String> typeAdapter;
    
    private Locomotive currentLoco;

    private boolean currentLocoIsNew;

    private Accessory currentAccy;

    private boolean currentAccyIsNew;

    private Dialog locoDialog;
    
    private Dialog accyDialog;
    
    private MenuItem menuConnect;
    
    private boolean connected;
    
    private MenuItem menuPower;
    
    private boolean power;
    
    private MenuItem menuVoice;
    
    private boolean voice;
    
    private Intent intent;

    private SpeechRecognizer recognizer;

    private BluetoothDevice bluetoothDevice;
    
    private Railuino railuino;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.main);

        // Get references to table rows
        for (int i = 0; i < 10; i++) {
            tableRows[i] = (TableRow)findViewById(getResources().getIdentifier("tableRow" + i, "id", getPackageName()));
        }
        
        // Get references to locomotive UI elements
        locoSpinner = (Spinner)findViewById(R.id.locoSpinner);
        locoConfigButton = (ImageButton)findViewById(R.id.locoConfigButton);
        locoImageView = (ImageView)findViewById(R.id.locoImage);
        locoSpeedBar = (SeekBar)findViewById(R.id.locoSpeedBar);
        locoSlowerButton = (ImageButton)findViewById(R.id.locoSlowerButton);
        locoDirectButton = (ImageButton)findViewById(R.id.locoDirectButton);
        locoFasterButton = (ImageButton)findViewById(R.id.locoFasterButton);
        
        for (int i = 0; i < 10; i++) {
            locoFunctionButtons[i] = (ToggleButton)findViewById(getResources().getIdentifier("locoF" + i + "Button", "id", getPackageName()));
        }

        // Get references to accessory UI elements
        accySpinner = (Spinner)findViewById(R.id.accySpinner);
        accyConfigButton = (ImageButton)findViewById(R.id.accyConfigButton);

        for (int i = 0; i < 10; i++) {
            accyButtons[i] = (ImageButton)findViewById(getResources().getIdentifier("accy" + i + "Button", "id", getPackageName()));
        }
        
        // Initialize adapters for spinners
        locoAdapter = new ArrayAdapter(this, R.layout.spinner);
        locoAdapter.setDropDownViewResource(android.R.layout.simple_dropdown_item_1line);
        locoSpinner.setAdapter(locoAdapter);
        
        accyAdapter = new ArrayAdapter(this, R.layout.spinner);
        accyAdapter.setDropDownViewResource(android.R.layout.simple_dropdown_item_1line);
        accySpinner.setAdapter(accyAdapter);

        typeAdapter = new ArrayAdapter(this, R.layout.spinner);
        typeAdapter.setDropDownViewResource(android.R.layout.simple_dropdown_item_1line);
        typeAdapter.add(getString(R.string.mm2));
        typeAdapter.add(getString(R.string.dcc));
        typeAdapter.add(getString(R.string.mfx));

        Locomotive.untitled = getString(R.string.newLoco);
        Accessory.untitled = getString(R.string.newAccy);

        load();
        
        locoAdapter.add(new Locomotive());
        accyAdapter.add(new Accessory());
        
        // Connect locomotive listeners
        locoSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                currentLoco = locoAdapter.getItem(arg2);
                currentLocoIsNew = arg2 == locoAdapter.getCount() - 1;
                
                refreshLoco();
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        
        locoConfigButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                showDialog(DIALOG_LOCO);
            }
        });
        
        locoConfigButton.setOnLongClickListener(new OnLongClickListener() {
           @Override
            public boolean onLongClick(View v) {
               if (!currentLocoIsNew) {
                   showDialog(DIALOG_LOCO_DELETE);
               }
               return true;
            } 
        });
        
        locoImageView.setOnLongClickListener(new OnLongClickListener() {
           @Override
            public boolean onLongClick(View v) {
               Intent intent = new Intent(Intent.ACTION_PICK, 
                       android.provider.MediaStore.Images.Media.INTERNAL_CONTENT_URI);
                    
                   intent.setType("image/*");
                   startActivityForResult(intent, 1);
                return true;
            } 
        });
        
        locoSlowerButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                locoSpeedBar.incrementProgressBy(-77);
                
                Log.d("XXX", "w=" + v.getWidth() + " h=" + v.getHeight());
            }
        });

        locoDirectButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                locoSpeedBar.setProgress(0);
            }
        });

        locoDirectButton.setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                int d = 1 - currentLoco.getDirection();
                currentLoco.setDirection(d);
                locoDirectButton.setImageResource(d == 0 ? R.drawable.forward : R.drawable.back);

                if (railuino != null) {
                    railuino.setLocoDirection(currentLoco.getFullAddress(), d);
                }
                
                locoSpeedBar.setProgress(0);
                
                return true;
            }
        });

        locoFasterButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                locoSpeedBar.incrementProgressBy(77);
            }
        });
        
        locoSpeedBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (railuino != null) {
                    railuino.setLocoSpeed(currentLoco.getFullAddress(), locoSpeedBar.getProgress());
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                currentLoco.setSpeed(progress);
                
                if (!fromUser && railuino != null) {
                    railuino.setLocoSpeed(currentLoco.getFullAddress(), progress);
                }
            }
        });

        for (int i = 0; i < 10; i++) {
            locoFunctionButtons[i].setTag(Integer.valueOf(i));
            locoFunctionButtons[i].setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int i = (Integer)v.getTag();
                    boolean b = ((ToggleButton)v).isChecked();
                    
                    currentLoco.setFunction(i, b);
                    
                    if (railuino != null) {
                        railuino.setLocoFunction(currentLoco.getFullAddress(), i, b);
                    }
                }
            });
        }

        // Connect accessory listeners
        accySpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                currentAccy = accyAdapter.getItem(arg2);
                currentAccyIsNew = arg2 == accyAdapter.getCount() - 1;
                
                refreshAccy();
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        
        accyConfigButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                showDialog(DIALOG_ACCY);
            }
        });
        
        accyConfigButton.setOnLongClickListener(new OnLongClickListener() {
            @Override
             public boolean onLongClick(View v) {
                if (!currentAccyIsNew) {
                    showDialog(DIALOG_ACCY_DELETE);
                }
                return true;
             } 
         });

        // Connect accessory listeners
        for (int i = 0; i < 10; i++) {
            accyButtons[i].setTag(Integer.valueOf(i));
            accyButtons[i].setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int i = (Integer)v.getTag();
                    boolean b = !currentAccy.getState(i);
                    currentAccy.setState(i, b);
                    accyButtons[i].setImageResource(b ? R.drawable.turn_straight : R.drawable.turn_round);
                    
                    if (railuino != null) {
                        railuino.setTurnout(currentAccy.getFullAddress() + i, b);
                    }
                }
            });
        }

        // Setup speech engine
        intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_WEB_SEARCH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, "de");
//        intent.putExtra(RecognizerIntent.EXTRA_CALLING_PACKAGE,
//                "de.pleumann.android.speechin");
        intent.putExtra(RecognizerIntent.EXTRA_PARTIAL_RESULTS, false); // false
        intent.putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 10);
//        intent.putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS, 500);
//        intent.putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_MINIMUM_LENGTH_MILLIS, 1000);
//        intent.putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, 1000);

        recognizer = SpeechRecognizer.createSpeechRecognizer(this);
        recognizer.setRecognitionListener(new RecognitionListener() {
            @Override
            public void onRmsChanged(float rmsdB) {
            }
            
            @Override
            public void onResults(Bundle results) {
                // recognizer.stopListening();
                
                Log.d("XXX", "onResults: " + results);

                ArrayList<String> strings = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION);
                float[] scores = results.getFloatArray(SpeechRecognizer.CONFIDENCE_SCORES);
                for (int i = 0; i < scores.length; i++) {
                    Log.d("XXX", strings.get(i) + " - score: " + scores[i]);
                }
                
                onVoiceCommand(strings);

                if (voice) {
                    try {
                        Thread.sleep(500);
                    } catch(InterruptedException e) {
                        // Ignored
                    }
                    
                    recognizer.startListening(intent);
                }
            }
            
            @Override
            public void onReadyForSpeech(Bundle params) {
                Log.d("XXX", "onReadyForSpeech");
            }
            
            @Override
            public void onPartialResults(Bundle partialResults) {
                Log.d("XXX", "onPartialResults");
            }
            
            @Override
            public void onEvent(int eventType, Bundle params) {
                Log.d("XXX", "onEvent");
            }
            
            @Override
            public void onError(int error) {
                Log.d("XXX", "onError");
                
                if (voice) {
                    try {
                        Thread.sleep(500);
                    } catch(InterruptedException e) {
                        // Ignored
                    }
                    
                    recognizer.startListening(intent);
                }
            }
            
            @Override
            public void onEndOfSpeech() {
                Log.d("XXX", "onEndOfSpeech");
            }
            
            @Override
            public void onBufferReceived(byte[] buffer) {
                Log.d("XXX", "onBufferReceived");
            }
            
            @Override
            public void onBeginningOfSpeech() {
                Log.d("XXX", "onBeginningOfSpeech");
            }
        });
        
        // Setup Bluetooth stuff
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> devices = adapter.getBondedDevices();
        for (BluetoothDevice d: devices) {
            if ("linvor".equals(d.getName())) {
                Log.d("XXX", "Found BT device");
                bluetoothDevice = d;
                break;
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        
        menuConnect = menu.findItem(R.id.menu_connect);
        menuPower = menu.findItem(R.id.menu_power);
        menuVoice = menu.findItem(R.id.menu_voice);
        
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item == menuConnect) {
            
            if (!connected) {
                try {
                    railuino = new Railuino(bluetoothDevice);
        
                    connected = true;
                    menuConnect.setIcon(R.drawable.bluetooth_connected);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                railuino.close();
                railuino = null;
                
                connected = false;
                menuConnect.setIcon(R.drawable.bluetooth_disconnected);
            }
            
            return true;
        } else if (item == menuPower) {
         
            power = !power;
            menuPower.setIcon(power ? R.drawable.power_on_2: R.drawable.power_off_2);
            
            if (railuino != null) {
                railuino.setPower(power);
            }
            
            return true;
        } else if (item == menuVoice) {

            voice = !voice;
            menuVoice.setIcon(voice ? R.drawable.mic_on_2: R.drawable.mic_off_2);
            
            if (voice) {
                recognizer.startListening(intent);
            } else {
                recognizer.stopListening();
            }
            
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_LOCO) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            
            View view = View.inflate(this, R.layout.loco, null);
            
            builder.setTitle(R.string.edit_loco);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    EditText labelField = (EditText)locoDialog.findViewById(R.id.labelField);
                    Spinner typeSpinner = (Spinner)locoDialog.findViewById(R.id.typeSpinner);
                    EditText addressField = (EditText)locoDialog.findViewById(R.id.addressField);
                    
                    currentLoco.setName(labelField.getText().toString());
                    currentLoco.setType(typeSpinner.getSelectedItemPosition());
                    currentLoco.setAddress(Integer.parseInt(addressField.getText().toString()));

                    if (currentLocoIsNew) {
                        currentLoco.setPhoto(new File(getFilesDir(), "" + Long.toHexString(System.currentTimeMillis()) + ".jpg"));
                        locoAdapter.add(new Locomotive());
                        currentLocoIsNew = false;
                    }
                    
                    save();
                    locoAdapter.notifyDataSetChanged();
                    refreshLoco();
                }
            });
            builder.setNegativeButton(android.R.string.cancel, null);
            builder.setView(view);
            
            ((Spinner)view.findViewById(R.id.typeSpinner)).setAdapter(typeAdapter);
            
            locoDialog = builder.create();
            
            return locoDialog;
        } else if (id == DIALOG_ACCY) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            
            View view = View.inflate(this, R.layout.loco, null);
            
            builder.setTitle(R.string.edit_accy);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    EditText labelField = (EditText)accyDialog.findViewById(R.id.labelField);
                    Spinner typeSpinner = (Spinner)accyDialog.findViewById(R.id.typeSpinner);
                    EditText addressField = (EditText)accyDialog.findViewById(R.id.addressField);
                    
                    currentAccy.setName(labelField.getText().toString());
                    currentAccy.setType(typeSpinner.getSelectedItemPosition());
                    currentAccy.setAddress(Integer.parseInt(addressField.getText().toString()));
                    
                    if (currentAccyIsNew) {
                        accyAdapter.add(new Accessory());
                        currentAccyIsNew = false;
                    }
                    
                    save();
                    accyAdapter.notifyDataSetChanged();
                    refreshAccy();
                }
            });
            builder.setNegativeButton(android.R.string.cancel, null);
            builder.setView(view);
            
            ((Spinner)view.findViewById(R.id.typeSpinner)).setAdapter(typeAdapter);
            
            accyDialog = builder.create();
            
            return accyDialog;
        } else if (id == DIALOG_LOCO_DELETE) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            
            builder.setTitle(R.string.delete_loco_title);
            builder.setMessage(R.string.delete_loco_message);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    locoAdapter.remove(currentLoco);
                    int arg2 = locoSpinner.getSelectedItemPosition();
                    currentLoco = locoAdapter.getItem(arg2);
                    currentLocoIsNew = arg2 == locoAdapter.getCount() - 1;
                    refreshLoco();
                    save();
                }
            });
            builder.setNegativeButton(android.R.string.cancel, null);
            
            return builder.create();

        } else if (id == DIALOG_ACCY_DELETE) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            
            builder.setTitle(R.string.delete_accy_title);
            builder.setMessage(R.string.delete_accy_message);
            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    accyAdapter.remove(currentAccy);
                    int arg2 = accySpinner.getSelectedItemPosition();
                    currentAccy = accyAdapter.getItem(arg2);
                    currentAccyIsNew = arg2 == accyAdapter.getCount() - 1;
                    refreshAccy();
                    save();
                }
            });
            builder.setNegativeButton(android.R.string.cancel, null);
            
            return builder.create();

        }
        
        return super.onCreateDialog(id);
    }
    
    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        if (id == DIALOG_LOCO) {
            EditText labelField = (EditText)locoDialog.findViewById(R.id.labelField);
            Spinner typeSpinner = (Spinner)locoDialog.findViewById(R.id.typeSpinner);
            EditText addressField = (EditText)locoDialog.findViewById(R.id.addressField);
            
            labelField.setText(currentLoco.getName());
            typeSpinner.setSelection(currentLoco.getType());
            addressField.setText("" + currentLoco.getAddress());
            
            return;
        } else if (id == DIALOG_ACCY) {
            EditText labelField = (EditText)accyDialog.findViewById(R.id.labelField);
            Spinner typeSpinner = (Spinner)accyDialog.findViewById(R.id.typeSpinner);
            EditText addressField = (EditText)accyDialog.findViewById(R.id.addressField);
            
            labelField.setText(currentAccy.getName());
            typeSpinner.setSelection(currentAccy.getType());
            addressField.setText("" + currentAccy.getAddress());
            
            return;
        }

        
        super.onPrepareDialog(id, dialog);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode != RESULT_OK) {
            return;
        }
     
        if (requestCode == 1) {
                final Intent intent = new Intent("com.android.camera.action.CROP");
                intent.setData(data.getData());
                intent.putExtra("outputX", 720);
                intent.putExtra("outputY", 240);
                intent.putExtra("aspectX", 3);
                intent.putExtra("aspectY", 1);
                intent.putExtra("scale", true);
                intent.putExtra("noFaceDetection", true);
                // intent.putExtra("return-data", true);
                intent.putExtra("output", Uri.fromFile(new File("/sdcard/railuino-tmp.jpg")));
                
                startActivityForResult(intent, 2);
        } else if (requestCode == 2) {
            currentLoco.getPhoto().delete();
            boolean b = new File("/sdcard/railuino-tmp.jpg").renameTo(currentLoco.getPhoto());
            if (!b) {
                copyFile(new File("/sdcard/railuino-tmp.jpg"), currentLoco.getPhoto());
            }
            Log.d("XXX", "Rename: " + b);
            refreshLoco();
            
            //try {
//                Bitmap photo = BitmapFactory.decodeFile("/sdcard/Download/tmp.jpg");
//                
//                Log.d("XXX", "w=" + photo.getWidth() + " h=" + photo.getHeight());
                
                // Bitmap photo = MediaStore.Images.Media.getBitmap(this.getContentResolver(), data.getData());
                // Bitmap photo = data.getParcelableExtra("data");
//                currentLoco.setPhoto(photo);
//                locoImageView.setImageBitmap(photo);
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
        }
    }
    
    private void refreshLoco() {
        if (currentLocoIsNew) {
            for (int i = 2; i < 7; i++) {
                tableRows[i].setVisibility(View.INVISIBLE);
            }
        } else {
            for (int i = 2; i < 7; i++) {
                tableRows[i].setVisibility(View.VISIBLE);
            }
           
            Log.d("XXX", "Bitmap is: " + currentLoco.getPhoto());
            Bitmap photo = BitmapFactory.decodeFile(currentLoco.getPhoto().getAbsolutePath());
            if (photo != null) {
                locoImageView.setImageBitmap(photo);
            } else {
                locoImageView.setImageResource(R.drawable.loco);
            }
            
            locoSpeedBar.setProgress(currentLoco.getSpeed());
            
            locoDirectButton.setImageResource(currentLoco.getDirection() == 0 ? R.drawable.forward : R.drawable.back);

            for (int i = 0; i < 10; i++) {
                locoFunctionButtons[i].setChecked(currentLoco.getFunction(i));
            }
        }
    }
    
    private void refreshAccy() {
        if (currentAccyIsNew) {
            tableRows[8].setVisibility(View.INVISIBLE);
            tableRows[9].setVisibility(View.INVISIBLE);
        } else {
            tableRows[8].setVisibility(View.VISIBLE);
            tableRows[9].setVisibility(View.VISIBLE);
            
            for (int i = 0; i < 10; i++) {
                accyButtons[i].setImageResource(currentAccy.getState(i) ? R.drawable.turn_straight : R.drawable.turn_round);
            }
        }
    }
    
    private void onVoiceCommand(ArrayList<String> results) {
        for (String s: results) {
            if (matches(getString(R.string.voice_slower), s)) {
                locoSlowerButton.performClick();
                return;
            }
            
            if (matches(getString(R.string.voice_faster), s)) {
                locoFasterButton.performClick();
                return;
            }
            
            if (matches(getString(R.string.voice_stop), s)) {
                locoSpeedBar.setProgress(0);
                return;
            }

            if (matches(getString(R.string.voice_low_speed), s)) {
                locoSpeedBar.setProgress(250);
                return;
            }
            
            if (matches(getString(R.string.voice_half_speed), s)) {
                locoSpeedBar.setProgress(500);
                return;
            }
            
            if (matches(getString(R.string.voice_high_speed), s)) {
                locoSpeedBar.setProgress(750);
                return;
            }

            if (matches(getString(R.string.voice_full_speed), s)) {
                locoSpeedBar.setProgress(1000);
                return;
            }

            if (matches(getString(R.string.voice_direction), s)) {
                locoDirectButton.performClick();
                return;
            }

            if (matches(getString(R.string.voice_forward), s)) {
                if (currentLoco.getDirection() != 0) {
                    locoDirectButton.performClick();
                }
                return;
            }

            if (matches(getString(R.string.voice_backward), s)) {
                if (currentLoco.getDirection() != 1) {
                    locoDirectButton.performClick();
                }
                return;
            }
            
            if (matches(getString(R.string.voice_lights), s)) {
                locoFunctionButtons[0].performClick();
                return;
            }
            
            for (int i = 0; i < 10; i++) {
                if (matches("F" + i, s) || matches(getString(R.string.voice_function) + " " + i, s)) {
                    locoFunctionButtons[i].performClick();
                    return;
                }
            }
            
            for (int i = 0; i < locoAdapter.getCount(); i++) {
                if (matches(s, locoAdapter.getItem(i).getName())) {
                    locoSpinner.setSelection(i);
                    return;
                }
            }

            for (int i = 0; i < 10; i++) {
                if (matches(getString(R.string.voice_turnout) + " " + i, s)) {
                    accyButtons[i].performClick();
                    return;
                }
            }
            
            for (int i = 0; i < accyAdapter.getCount(); i++) {
                if (matches(s, accyAdapter.getItem(i).getName())) {
                    accySpinner.setSelection(i);
                    return;
                }
            }
        }
    }
    
    private boolean matches(String a, String b) {
        String aa = a.replaceAll(" ", "");
        String bb = b.replaceAll(" ", "");
        
        boolean result = aa.equalsIgnoreCase(bb);
        
        if (result) {
            Toast.makeText(this, a, Toast.LENGTH_SHORT).show();
        }
        
        return result;
    }
    
    public void save() {
        SharedPreferences.Editor prefs = getSharedPreferences("config", Context.MODE_PRIVATE).edit();
        prefs.clear();

        prefs.putInt("loco.count", locoAdapter.getCount() - 1);
        for (int i = 0; i < locoAdapter.getCount() - 1; i++) {
            Locomotive loco = locoAdapter.getItem(i);
            
            prefs.putString("loco." + i + ".name", loco.getName());
            prefs.putInt("loco." + i + ".type", loco.getType());
            prefs.putInt("loco." + i + ".address", loco.getAddress());
            prefs.putString("loco." + i + ".image", loco.getPhoto().getAbsolutePath());
        }
        
        prefs.putInt("accy.count", accyAdapter.getCount() - 1);
        for (int i = 0; i < accyAdapter.getCount() - 1; i++) {
            Accessory accy = accyAdapter.getItem(i);
            
            prefs.putString("accy." + i + ".name", accy.getName());
            prefs.putInt("accy." + i + ".type", accy.getType());
            prefs.putInt("accy." + i + ".address", accy.getAddress());
        }

        prefs.commit();
    }
    
    public void load() {
        SharedPreferences prefs = getSharedPreferences("config", Context.MODE_PRIVATE);

        int numLoco = prefs.getInt("loco.count", 0);
        for (int i = 0; i < numLoco; i++) {
            Locomotive loco = new Locomotive();
            loco.setName(prefs.getString("loco." + i + ".name", ""));
            loco.setType(prefs.getInt("loco." + i + ".type", 0));
            loco.setAddress(prefs.getInt("loco." + i + ".address", 0));
            loco.setPhoto(new File(prefs.getString("loco." + i + ".image", "")));
            
            locoAdapter.add(loco);
        }

        int numAccy = prefs.getInt("accy.count", 0);
        for (int i = 0; i < numAccy; i++) {
            Accessory accy = new Accessory();
            accy.setName(prefs.getString("accy." + i + ".name", ""));
            accy.setType(prefs.getInt("accy." + i + ".type", 0));
            accy.setAddress(prefs.getInt("accy." + i + ".address", 0));
            
            accyAdapter.add(accy);
        }
    }
    
    public void copyFile(File from, File to) {
        try {
            FileInputStream fis = new FileInputStream(from);
            FileOutputStream fos = new FileOutputStream(to);
            
            byte[] buffer = new byte[8192];
            int i = fis.read(buffer);
            while (i >= 0) {
                fos.write(buffer, 0, i);
                i = fis.read(buffer);
            }
            fos.flush();
            fos.close();
            fis.close();
        } catch (IOException e) {
            Log.e("XXX", "Error copying file", e);
        }
    }
     
}
