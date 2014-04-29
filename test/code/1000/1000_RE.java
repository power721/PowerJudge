import java.io.*;
import java.util.*;

public class Main
{
  public static void main(String[] args)
  {
    Scanner cin = new Scanner(new BufferedInputStream(System.in));
    int a, b;
    String s = "123";
    while (cin.hasNext())
    {
      a = cin.nextInt();
      b = cin.nextInt();
      System.out.println(a+b);
      s = null;
    }
    if (s.equals("123"))
      System.out.println(s);
  }
}