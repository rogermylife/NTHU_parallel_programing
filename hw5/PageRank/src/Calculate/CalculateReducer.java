package pagerank;

import java.io.IOException;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

public class CalculateReducer extends Reducer<Text,Text,Text,Text> {

    private double danglingWeights;
    private double danglingPR;
    private double err;
    private long N;
    private static final double alpha = 0.85;

    @Override
    protected void setup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        this.N = context.getConfiguration().getLong("N",0L);
        this.danglingPR = context.getConfiguration().getDouble("danglingWeights", 0.0);
        this.danglingPR = this.danglingPR/N*alpha;
        this.danglingWeights =0.0;
        this.err = 0.0;

        System.out.println("N "+ this.N + " danglingPR " + this.danglingPR);
    }
	
    public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {

        boolean isDanglingNode = false;
        JSONObject status = new JSONObject();
        double linkScore;
        double score = 0.0;
        System.out.println(score);
		for (Text val: values) {
            JSONObject linkScoreJSON = new JSONObject(val.toString());
            try {
                linkScore = linkScoreJSON.getDouble("linkScore");
            }catch (JSONException e)
            {
                status = linkScoreJSON;
                if(status.getLong("outDegree") == 0)
                    isDanglingNode = true;
                continue;
            }
            System.out.println(score +" + "+ linkScore);
            score += linkScore;
        }

        System.out.println("MY score " + key.toString() + " " +status.getDouble("score"));
        // links score
        System.out.println(score +" * "+ alpha);
        score *= alpha;
        // random score
        System.out.println(score +" + "+ (1.0-alpha)/N);
        score += (1.0-alpha)/N;
        // dangling score
        System.out.println(score +" + "+ danglingPR);
        score += danglingPR;

        // calculate err
        err+= Math.abs(status.getDouble("score")-score);
        System.out.println("err " + status.getDouble("score") + " - " + score +" abs " + Math.abs(status.getDouble("score")-score));

        System.out.println("put score "+score);
        status.put("score", score);
		context.write(key, new Text(status.toString()));

        if(isDanglingNode)
            danglingWeights += score;
	}

    @Override
    protected void cleanup(Reducer<Text, Text, Text, Text>.Context context) throws IOException, InterruptedException {
        context.getCounter("N", "danglingWeights").increment((long)( danglingWeights*Long.MAX_VALUE ));
        context.getCounter("N", "err").increment((long)( err *Long.MAX_VALUE ));
    }
}
