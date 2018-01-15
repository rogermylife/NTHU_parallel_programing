package pagerank;

import java.io.IOException;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;
import org.json.JSONObject;
import org.json.JSONArray;

public class ParseReducer extends Reducer<Text,Text,Text,Text> {

    private long N;

    @Override
    protected void setup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        this.N = context.getCounter("N", "N").getValue();
    }
	
    public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {

        String linkString = key.toString();
        boolean isMissLink = true;
        JSONObject status = new JSONObject();
        JSONArray titles = new JSONArray();
        //String status = "{{" + (double)1.0 + "}}";
		for (Text val: values) {
            if( val.getLength() == 0 )
            {
                isMissLink = false;
                continue;
            }
            String titleString = val.toString();
            titles.put(titleString);
        }
        status.put("titles", titles);
        if(!isMissLink)
            context.write(key, new Text(status.toString()));
    }

    @Override
    protected void cleanup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        System.out.println("N "+context.getCounter("N", "N").getValue());
    }
}
