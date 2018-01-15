package pagerank;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

public class TextDoublePair implements Writable, WritableComparable<TextDoublePair> {
    public Text text;
    public double score;

    public TextDoublePair(){
        this(new Text(), 0.0);
    }
    public TextDoublePair(Text text, double score)
    {
        this.text = text;
        this.score = score;
    }

    @Override
    public void write(DataOutput out) throws IOException{
        text.write(out);
        out.writeDouble(score);
    }

    @Override
    public void readFields(DataInput in) throws IOException{
        text.readFields(in);
        score = in.readDouble();
    }

    @Override
    public String toString() {
        return "("+ text.toString() +","+ score +")";
    }

    @Override
    public int compareTo(TextDoublePair o)
    {
        return this.text.compareTo(o.text);
    }
}
