import java.util.Scanner

fun main(args: Array<String>) {
    var a : Int
    var b : Int
    val input = Scanner(System.`in`)
    while (input.hasNext()) {
        a = input.nextInt()
        b = input.nextInt()
        println(a + b)
    }
}
