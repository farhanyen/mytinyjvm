public class InstanceMethod {
        public int add(int x, int y) {
            return x + y;
        }
        public static void main(String args[]) {
            InstanceMethod m = new InstanceMethod();
            int x = m.add(1, 2);
//             System.out.println(x);
        }
}