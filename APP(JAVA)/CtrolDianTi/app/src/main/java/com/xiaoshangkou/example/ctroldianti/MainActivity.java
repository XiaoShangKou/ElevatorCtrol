package com.xiaoshangkou.example.ctroldianti;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.select.Elements;
import org.w3c.dom.ls.LSOutput;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    private Handler handler;
    private  String value;
    private Spinner spinner;
    private Button button;
    private ProgressBar progressBar;
    private Button shuoming;

    /*汉语*/
    final String[] Chinese=new String[]{
            "1楼","2楼","3楼","4楼","5楼","6楼",
            "预约",
            "预约成功","确定", "网络原因预约失败",
            "使用说明",
            "请点击要去往的楼层"};
    final   String[] strings=new String[]{"汉语","藏语"};
    /*藏语*/
    final String[] Zangyu=new String[]{
            "ཐོག་རྩེ་དང་པོ་།","ཐོག་རྩེ་གཉིས་པ་།། ","ཐོག་རྩེ་གསུམ་པ་།། ","ཐོག་རྩེ་བཞི་པ་།།","ཐོག་རྩེ་ལྔ་པ་།།", "ཐོག་རྩེ་དྲུག་པ་།།",
            "སྔོན་ནས་མངགས་པ་།།",
            "ཁ་བཅད་ལེགས་འགྲུབ་བྱུང་བ་།།","གཏེན་འཁེལ་བ་།།","ཁ་བཅད་ལེགས་འགྲུབ་མ་བྱུང་བ་།།",
            "བཀོལ་སྤྱོད་གསལ་བཤད།",
            "འགྲོ་དགོས་པའི་ཐོག་ཁང་ལ་གནོན་རོགས་།"};
    final String[] strings2=new String[]{"བོད་ཡིག་།","རྒྱ་ཡིག་།།"};//藏语，汉语

    String chenggong;//预约成功提示框的内容
    String tishi;//没有选择楼层时的提示内容
    String shibai;//预约失败提示框的内容
    String sure;//提示框的确定按钮

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        progressBar=findViewById(R.id.jiazai);
        progressBar.setVisibility(View.GONE);
        shuoming=findViewById(R.id.shuoming);

        //点击使用说明按钮，跳转页面
        shuoming.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent1=new Intent(MainActivity.this, ShuoMing.class);
                //启动Activity
                startActivity(intent1);
            }
        });

        //适配器
        //第二个参数为列表项的样式
        ArrayAdapter<String> arrayAdapter=new ArrayAdapter<>(this, R.layout.simple_spinner_item,strings);
        //设置列表项下拉时的选项样式
        arrayAdapter.setDropDownViewResource(android.R.layout.simple_spinner_item);
        spinner=findViewById(R.id.spinner);
        //将适配器，和下拉列表框关联起来
        spinner.setAdapter(arrayAdapter);

        /*切换那种语言(藏语/汉语)*/
        //获取用户选在下拉列表框的值
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                //获取用户选在下拉列表框的值
                value=(String) spinner.getSelectedItem();
                if("汉语".equals(value) || "རྒྱ་ཡིག་།།".equals(value) ){
                    language(Chinese,strings);
                }
                else if("藏语".equals(value) ||"བོད་ཡིག་།".equals(value))
                {
                    language(Zangyu,strings2);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });


        //得到Button对象
        button=findViewById(R.id.bd);
        //得到RadioGroup对象
        RadioGroup radioGroup=findViewById(R.id.rg);

        handler=new Handler(new Handler.Callback() {
            @Override
            public boolean handleMessage(@NonNull Message message) {
                if (message.what==0x110){
                    int Sucess=message.arg1;
                    if(Sucess==0){

                        AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
                        builder.setMessage(chenggong);
                        builder.setPositiveButton(sure,null).show();
                        progressBar.setVisibility(View.GONE);
                    }else if(Sucess==1){
                        AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
                        builder.setMessage(shibai);
                        builder.setPositiveButton(sure,null).show();
                        progressBar.setVisibility(View.GONE);

                    }
                    else {
                        AlertDialog.Builder builder=new AlertDialog.Builder(MainActivity.this);
                        builder.setMessage("未知错误");
                        builder.setPositiveButton(sure,null).show();

                    }
                }
                return false;
            }
        });

        //预约按钮点击点击事件
        button.setOnClickListener(new View.OnClickListener() {
            
            @Override
            public void onClick(View view) {
                int noChecksum = 0;//没有点击楼层的个数
                for (int i=0;i<radioGroup.getChildCount();i++){
                    if (radioGroup.getChildAt(i) instanceof RadioButton) {
                        RadioButton radioButton = (RadioButton) radioGroup.getChildAt(i);
                        if (radioButton.isChecked()){
                            progressBar.setVisibility(View.VISIBLE);
                            Thread t1 = new Thread(new Thread1(handler,(i+1)));
                            t1.setName("通行");
                            //启动线程
                            t1.start();

                            break;
                        }
                        else{
                            //用户没有点击楼层就按下预约按钮，那么提示用户要按楼层
                            noChecksum++;
                            if(noChecksum==6) {
                                Toast.makeText(MainActivity.this, tishi, Toast.LENGTH_SHORT).show();
                            }
                        }
                    }

                }
            }
        });

    }
    /*
    * 用途:界面的文字部署，这样可以支持语言的切换，目前支持汉文，藏文的切换
    * 参数:语言内容的数组,spinner组件内容的数组
    * 返回值:无
    * 时间:2022/5/1
    * 修改:暂无
    * */
    void language(String[] yuyan,String[] strings){

        //获取组件
        RadioButton radioButton1=findViewById(R.id.rb_a);
        RadioButton radioButton2=findViewById(R.id.rb_b);
        RadioButton radioButton3=findViewById(R.id.rb_c);
        RadioButton radioButton4=findViewById(R.id.rb_d);
        RadioButton radioButton5=findViewById(R.id.rb_e);
        RadioButton radioButton6=findViewById(R.id.rb_f);

        //设置内容
        radioButton1.setText(yuyan[0]);
        radioButton2.setText(yuyan[1]);
        radioButton3.setText(yuyan[2]);
        radioButton4.setText(yuyan[3]);
        radioButton5.setText(yuyan[4]);
        radioButton6.setText(yuyan[5]);

        //spinner组件内容的切换
        ArrayAdapter<String> arrayAdapter=new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item,strings);
        //将适配器，和下拉列表框关联起来
        spinner.setAdapter(arrayAdapter);

        button.setText(yuyan[6]);

        //提示框
        chenggong=yuyan[7];
        shibai=yuyan[9];
        sure=yuyan[8];

        //Toast内容语言的切换
        shuoming.setText(yuyan[10]);
        tishi=yuyan[11];

    }
}

/**
 * 用途:建立一个线程爬取esp32的web内容
 */
class Thread1 implements Runnable{


    private Handler handler;
    private int LouCen;//要去往的楼层
    public Thread1(Handler handler,int LouCen) {

        this.handler=handler;
        this.LouCen=LouCen;
    }

    @RequiresApi(api = Build.VERSION_CODES.N)
    @Override
    public void run() {
        String url="http://192.168.4.1/ctrol?IO="+LouCen;//网址

        Document doc = null;
        //实例化一个消息对象
        Message m=new Message();

        //发送结束消息0x110
        m.what=0x110;

        //开始get请求
        try {
            doc = Jsoup.connect(url)
                    .data("query", "Java")
                    .userAgent("Mozilla")
                    .cookie("auth", "token")
                    .timeout(5000)
                    .post();
//            Elements links2=doc.select("b");
            m.arg1=0;

        } catch (IOException e) {
            m.arg1=1;
            e.printStackTrace();
        }

        //定义完之后发送一个消息
        handler.sendMessage(m);

    }




}
