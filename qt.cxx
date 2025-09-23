class Vector {
    pvt int[] data;
    pvt int count;
    pvt int capacity;

    Self(int capacity) {
        this.count = 0;
        this.capacity = capacity;
    }

    ~Self() {
        delete data;
    }

    
}