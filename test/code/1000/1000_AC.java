import java.io.*;
import java.util.*;

public class Main
{
  /**
   * 1000题： a+b
   * 求两个整数的和
   */
  public static void main(String[] args)
  {
    Scanner 输入 = new Scanner(new BufferedInputStream(System.in));
    int 数字1;
    int 数字2;
    while(输入.hasNext())
    {
      数字1 = 输入.nextInt();
      数字2 = 输入.nextInt();
      输出(数字1 + 数字2);
    }
  }

  private static void 输出(int 数字)
  {
    System.out.println(数字);
  }
}