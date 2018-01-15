package pagerank;

import java.io.IOException;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;
import org.json.JSONObject;
import org.json.JSONArray;

public class InitReducer extends Reducer<Text,Text,Text,Text> {

    private long N;
    private long danglingWeights=0L;

    @Override
    protected void setup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        this.N = context.getConfiguration().getLong("N", 0L);
        this.danglingWeights = 0;
    }
	
    public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {

        String titleString = key.toString();
        boolean isDanglingNode = true;
        long outDegree=0L;
        JSONObject status = new JSONObject();
        JSONArray links = new JSONArray();
        //String status = "{{" + (double)1.0 + "}}";
		for (Text val: values) {
            if( val.getLength() == 0)
                continue;
            String linkString = val.toString();
            isDanglingNode = false;
            links.put(linkString);
            outDegree++;
        }
        status.put("score", (double)1.0/N);
        status.put("outDegree", outDegree);
        status.put("links", links);

        context.write(key, new Text(status.toString()) );

        if(isDanglingNode)
            ++danglingWeights;
    }

    @Override
    protected void cleanup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        context.getCounter("N", "danglingWeights").increment(danglingWeights);
    }
}
