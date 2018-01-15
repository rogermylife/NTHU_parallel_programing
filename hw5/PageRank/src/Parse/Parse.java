package pagerank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;


public class Parse{
	public Parse(){

  	}
	
	public Config parse(String[] args) throws Exception {
		Configuration conf = new Configuration();
		
		Job job = Job.getInstance(conf, "Parse");
		job.setJarByClass(Parse.class);

		// set the class of each stage in mapreduce
		job.setMapperClass(ParseMapper.class);
		job.setReducerClass(ParseReducer.class);
		
		// set the output class of Mapper and Reucer
		job.setMapOutputKeyClass(Text.class);
		job.setMapOutputValueClass(Text.class);
		job.setOutputKeyClass(Text.class);
		job.setOutputValueClass(Text.class);
		
		// add input/output path
		FileInputFormat.addInputPath(job, new Path(args[0]));
		FileOutputFormat.setOutputPath(job, new Path("Temp"+"/Parse"));
		
		job.waitForCompletion(true);

        long N = job.getCounters().findCounter("N", "N").getValue();
        return new Config(N, 0.0);
	}
	
}
