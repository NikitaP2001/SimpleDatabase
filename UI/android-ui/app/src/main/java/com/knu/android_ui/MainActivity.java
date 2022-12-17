package com.knu.android_ui;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.net.Socket;
import java.util.Iterator;

public class MainActivity extends AppCompatActivity {
    private final int serverPort = 1337;
    private final String serverIp = "172.27.96.1";
    private final String database = "tempdb";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btn = findViewById(R.id.main_btnConnect);

        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String command = "";
                        EditText query = findViewById(R.id.main_editQuery);
                        command = query.getText().toString();
                        String strJsCmd = packCommand(command, database);
                        JSONObject res = executeQuery(strJsCmd);
                        if (res != null) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    displayJson(res);
                                }
                            });
                        }
                    }
                }).start();
            }
        });
    }

    private String packCommand(String strCommand, String dbName) {
        JSONObject jo = new JSONObject();
        try {
            jo.put("command", strCommand);
            jo.put("database", dbName);
        } catch (JSONException ex) {
            String msg = ex.getMessage();
            if (msg != null)
                Log.d("ExecErr", msg);
        }
        return jo.toString();
    }

    protected JSONObject executeQuery(String message)
    {
        String output = "";
        JSONObject json = null;
        char[] sendBuf = new char[message.length() + 1];
        int result;
        try {
            Socket servSock = new Socket(serverIp, serverPort);
            PrintWriter writer = new PrintWriter(servSock.getOutputStream());
            BufferedReader reader = new BufferedReader(new InputStreamReader(servSock.getInputStream()));

            message.getChars(0, message.length(), sendBuf, 0);
            sendBuf[message.length()] = '\0';
            writer.write(sendBuf);
            writer.flush();
            do {
                result = reader.read(sendBuf);
                output = output + new String(sendBuf);
            } while (result > 0 && sendBuf[result-1] != '\0');
            if (result < 0)
                throw new IOException("readed recv error");

            json = new JSONObject(output);

        } catch (IOException ex) {
            String msg = ex.getMessage();
            if (msg != null)
                Log.d("ExecErr", msg);
            output = null;
        } catch (JSONException ex) {
        }
        return json;
    }

    private void displayJson(JSONObject table) {
        Iterator<String> keys = table.keys();

        TableLayout tl = (TableLayout) findViewById(R.id.main_table);
        tl.removeAllViewsInLayout();

        while(keys.hasNext()) {
            String key = keys.next();

            TableRow tr = new TableRow(this);
            tr.setLayoutParams(new TableRow.LayoutParams(TableRow.LayoutParams.FILL_PARENT, TableRow.LayoutParams.WRAP_CONTENT));

            TextView headLbl = new TextView(this);
            headLbl.setText(key);
            headLbl.setLayoutParams(new TableRow.LayoutParams(TableRow.LayoutParams.FILL_PARENT, TableRow.LayoutParams.WRAP_CONTENT));
            tr.addView(headLbl);
            try {
                 if (table.get(key) instanceof JSONArray) {
                    JSONArray jsonArray = table.getJSONArray(key);
                    for (int i = 0; i < jsonArray.length(); i++) {

                        String rowValue = jsonArray.getString(i);

                        TextView lbl = new TextView(this);
                        lbl.setText(rowValue);
                        lbl.setLayoutParams(new TableRow.LayoutParams(TableRow.LayoutParams.FILL_PARENT, TableRow.LayoutParams.WRAP_CONTENT));
                        tr.addView(lbl);
                    }
                }
            } catch (JSONException ex) {
                String msg = ex.getMessage();
                if (msg != null)
                    Log.d("ExecErr", msg);
            }
            tl.addView(tr);
        }
    }


}