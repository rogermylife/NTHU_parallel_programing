package pagerank;

public class Config
{
    public long N;
    public double danglingWeights;
    public double err;
    public int iteration;
    public Config(long N, double danglingWeights)
    {
        this(N, danglingWeights, 1.0, 0);
    }

    public Config(long N, double danglingWeights, double err)
    {
        this(N, danglingWeights, err, 0);
    }
    public Config(long N, double danglingWeights, double err, int iteration)
    {
        this.N = N;
        this.danglingWeights = danglingWeights;
        this.err = err;
        this.iteration = iteration;
    }
}
