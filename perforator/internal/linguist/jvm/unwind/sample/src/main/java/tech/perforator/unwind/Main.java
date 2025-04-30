package tech.perforator.unwind;

class Main {
    public static void main(String[] args) {
        for (int i = 0; i < 1; i++) {
            foo();
        }
    }

    static void foo() {
        for (int i = 0; i < 100000; i++) {
            new Main().bar(100000 - i - 1);
        }
    }

    void bar(int x) {
        Native.unwindIfZero0(x);
    }
}
