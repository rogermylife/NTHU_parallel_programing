package pagerank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.Set;
import java.util.List;

public class PageRank{
	
    public static void main(String[] args) throws Exception {  
    
    /* Don't need to modify this file.
       InputDir : args[0]
       TmpDir : args[1]
       OutputDir : args[2] 
       Number of reducer for Sort : args[3] */

    //Job 1: Parse link
    Parse parse = new Parse();
    Config config = parse.parse(args);

    //Job 2: Init score
    Init init = new Init();
    config = init.init(args, config);
    System.out.println("Init Done N "+config.N + " danglingWeights "+config.danglingWeights);

    //Job 3: Calculate
    while(config.iteration < Integer.parseInt(args[2]) && config.err > 0.001)
    {
        Calculate calculate = new Calculate();
        config = calculate.calculate(args, config);
        System.out.println("Calculate iter "+ config.iteration +" Done N "+config.N + " danglingWeights " + config.danglingWeights + " err " + config.err);
    }

    //Job 4: Sort
    Sort sort = new Sort();
    sort.sort(args, config);
    //job2.Sort(args);
    
    System.exit(0);
    }  
}
