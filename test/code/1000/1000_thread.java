import java.io.*;
import java.util.*;

public class Main
{
  public static void main(String[] args)
  {
    final Scanner cin = new Scanner(new BufferedInputStream(System.in));
    Thread judge = new Thread(new Runnable() {
    	@Override
    	public void run()
    	{
    		while(cin.hasNext())
		    {
		      int a = cin.nextInt();
		      int b = cin.nextInt();
		      System.out.println(a+b);
		    }
    	}
    });
    judge.start();
  }
}