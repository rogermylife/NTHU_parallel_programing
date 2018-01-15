package pagerank;

import java.io.IOException;

import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;

import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.conf.Configuration;

import java.net.URI; 
import java.io.*;

import org.json.JSONArray;
import org.json.JSONObject;


public class InitMapper extends Mapper<Text, Text, Text, Text> {

    private Text title = new Text("");
    private Text link = new Text("");
    private long N;
    
    @Override
    protected void setup(Mapper<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException{
        this.N = context.getConfiguration().getLong("N",0L);
    }
	
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException {

        context.write(key, new Text());
        JSONArray titles = new JSONObject(value.toString()).getJSONArray("titles");
        for(int i=0;i<titles.length();++i)
        {
            System.out.println("title " + titles.getString(i) +" link " + key.toString());
            context.write(new Text(titles.getString(i)), key);
        }
	}
	
}
