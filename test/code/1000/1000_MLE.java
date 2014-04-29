import java.io.*;
import java.util.*;

public class Main
{
  public static void main(String[] args)
  {
    Scanner cin = new Scanner(new BufferedInputStream(System.in));
    int a,b;
    String[] str = new String[50000000];
    str[0] = "";
    for (int i = 1; i < str.length; ++i)
    {
      str[i] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }
    while (cin.hasNext())
    {
      a = cin.nextInt();
      b = cin.nextInt();
      System.out.println(a+b);
    }
  }
}