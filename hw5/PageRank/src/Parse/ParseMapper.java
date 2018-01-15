package pagerank;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.io.LongWritable;
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



public class ParseMapper extends Mapper<LongWritable, Text, Text, Text> {

    private Text title = new Text("");
    private Text link = new Text("");
    private long N=0L;
	
	public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        
         	
		/*  Match title pattern */  
		Pattern titlePattern = Pattern.compile("<title>(.+?)</title>");
		// Matcher titleMatcher = titlePattern.matcher(xxx);
        Matcher titleMatcher = titlePattern.matcher(value.toString());
        if(titleMatcher.find())
        {
            //trim <title>ex <title>TAT</title> => TAT
            String titleString = titleMatcher.group(1);
            titleString = unescapeXML(titleString);
            // No need capitalizeFirstLetter
            title.set(titleString);
            ++N;
            context.write(title, new Text());

            /*  Match link pattern */
            Pattern linkPattern = Pattern.compile("\\[\\[(.+?)([\\|#]|\\]\\])");
            Matcher linkMatcher = linkPattern.matcher(value.toString());
            boolean notFound = true;
            while(linkMatcher.find())
            {
                notFound = false;
                String linkString = linkMatcher.group(1);
                linkString = unescapeXML(linkString);
                // Need capitalizeFirstLetter
                linkString = capitalizeFirstLetter(linkString);
                link.set(linkString);
                System.out.println(linkString +"<="+ titleString);
                context.write(link, title);
            }

        }
		
	
	}
	
	private String unescapeXML(String input) {

		return input.replaceAll("&lt;", "<").replaceAll("&gt;", ">").replaceAll("&amp;", "&").replaceAll("&quot;", "\"").replaceAll("&apos;", "\'");

    }

    private String capitalizeFirstLetter(String input){

    	char firstChar = input.charAt(0);

        if ( firstChar >= 'a' && firstChar <='z'){
            if ( input.length() == 1 ){
                return input.toUpperCase();
            }
            else
                return input.substring(0, 1).toUpperCase() + input.substring(1);
        }
        else 
        	return input;
    }
    @Override
    protected void cleanup(Mapper<LongWritable, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        context.getCounter("N", "N").increment(N);
    }
}
