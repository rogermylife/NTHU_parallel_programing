package pagerank;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;

import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.conf.Configuration;

import java.util.ArrayList;
import java.util.Arrays;
import java.net.URI; 
import java.io.*;

import org.json.JSONObject;
import org.json.JSONArray;

public class CalculateMapper extends Mapper<Text, Text, Text, Text> {

    private Text title = new Text("");
    private Text link = new Text("");
    private long N=0L;
    private long danglingWeights = 0L;

    /*
    @Override
    protected void setup(Mapper<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        this.N = context.getConfiguration().getLong("N",0L);
    }
    */
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
        
        //origin
        context.write(key, value);

        JSONObject status = new JSONObject(value.toString());
        double score = status.getDouble("score");
        long outDegree = status.getLong("outDegree");
        JSONArray links = status.getJSONArray("links");
        for( int i=0;i<links.length();++i)
        {
            JSONObject linkScore = new JSONObject();
            linkScore.put("linkScore", score/outDegree);
            context.write(new Text(links.getString(i)), new Text(linkScore.toString()));
        }
	}

    @Override
    protected void cleanup(Mapper<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
    }
}
