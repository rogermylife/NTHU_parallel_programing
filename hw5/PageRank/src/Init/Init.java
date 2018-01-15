package pagerank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;


public class Init{
	public Init(){

  	}
	
	public Config init(String[] args, Config inputConfig) throws Exception {
		Configuration conf = new Configuration();
		conf.setLong("N", inputConfig.N);

		Job job = Job.getInstance(conf, "Init");
		job.setJarByClass(Init.class);

        job.setInputFormatClass(KeyValueTextInputFormat.class);

		// set the class of each stage in mapreduce
		job.setMapperClass(InitMapper.class);
        job.setReducerClass(InitReducer.class);
		
		// set the output class of Mapper and Reucer
		job.setMapOutputKeyClass(Text.class);
		job.setMapOutputValueClass(Text.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
		
		// add input/output path
		FileInputFormat.addInputPath(job, new Path("Temp"+"/Parse"));
		FileOutputFormat.setOutputPath(job, new Path("Temp"+"/Init"));
		
		job.waitForCompletion(true);

        double danglingPR = (double)job.getCounters().findCounter("N", "danglingWeights").getValue()/inputConfig.N/inputConfig.N*0.85;
        return new Config(inputConfig.N, danglingPR);
	}
	
}
