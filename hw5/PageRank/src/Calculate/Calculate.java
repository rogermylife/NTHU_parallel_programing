package pagerank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;


public class Calculate{
	public Calculate(){

  	}
	
	public Config calculate(String[] args, Config inputConfig) throws Exception {
		Configuration conf = new Configuration();
        conf.setLong("N", inputConfig.N);
        conf.setDouble("danglingWeights", inputConfig.danglingWeights);
        int iteration = inputConfig.iteration;
		
		Job job = Job.getInstance(conf, "Calculate");
		job.setJarByClass(Calculate.class);

        job.setInputFormatClass(KeyValueTextInputFormat.class);

		// set the class of each stage in mapreduce
		job.setMapperClass(CalculateMapper.class);
		job.setReducerClass(CalculateReducer.class);
		
		// set the output class of Mapper and Reucer
		job.setMapOutputKeyClass(Text.class);
		job.setMapOutputValueClass(Text.class);
		job.setOutputKeyClass(Text.class);
		job.setOutputValueClass(Text.class);
		
		// add input/output path
        if( inputConfig.iteration == 0)
            FileInputFormat.addInputPath(job, new Path("Temp"+"/Init"));
        else
            FileInputFormat.addInputPath(job, new Path("Temp"+"/Calculate/" + iteration));
		FileOutputFormat.setOutputPath(job, new Path("Temp"+"/Calculate/"+ (iteration+1) ));
		
		job.waitForCompletion(true);

        double danglingWeights = (double)job.getCounters().findCounter("N", "danglingWeights").getValue()/Long.MAX_VALUE;
        double err = (double)job.getCounters().findCounter("N", "err").getValue()/Long.MAX_VALUE;
        return new Config( inputConfig.N, danglingWeights, err, iteration+1);
	}
	
}
