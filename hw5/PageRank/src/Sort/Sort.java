package pagerank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;

public class Sort {
	
	public Sort(){

        }
	
	public void sort(String[] args, Config inputConfig) throws Exception {
		Configuration conf = new Configuration();
		
		Job job = Job.getInstance(conf, "Sort");
		job.setJarByClass(Sort.class);
		
		job.setInputFormatClass(KeyValueTextInputFormat.class);	
		
		// set the class of each stage in mapreduce
		job.setMapperClass(SortMapper.class);
		job.setReducerClass(SortReducer.class);
		
		// set the output class of Mapper and Reducer
		job.setMapOutputKeyClass(SortPair.class);
		job.setMapOutputValueClass(NullWritable.class);
		job.setOutputKeyClass(Text.class);
		job.setOutputValueClass(DoubleWritable.class);
		
		// set the number of reducer
		//job.setNumReduceTasks(Integer.valueOf(args[3]));
		
		// add input/output path
		FileInputFormat.addInputPath(job, new Path("Temp"+"/Calculate/"+inputConfig.iteration));
		FileOutputFormat.setOutputPath(job, new Path(args[1]));

		job.waitForCompletion(true);
	}
}
