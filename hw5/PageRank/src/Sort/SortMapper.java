package pagerank;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.io.NullWritable;

import org.json.JSONObject;

public class SortMapper extends Mapper<Text, Text, SortPair, NullWritable> {
	
	private SortPair sp;
	
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
		
        JSONObject status = new JSONObject(value.toString());
        sp = new SortPair(key, status.getDouble("score") );
        System.out.println("Mapper " + sp.getWord().toString() + " " + sp.getAverage());
		context.write(sp, NullWritable.get());
	}
	
}
