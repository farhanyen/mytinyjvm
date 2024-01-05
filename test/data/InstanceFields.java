public class InstanceFields {
        int a;
        int getA() {
            return a;
        }
        void setA(int v) {
            a = v;
        }
        public static void main(String args[]) {
            InstanceFields m = new InstanceFields();
            m.setA(5);
            int x = m.getA();
        }
}