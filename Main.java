import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Main {
    public static void main(final String[] args) throws Exception {
        List<Mat> matrices = new ArrayList<>();

        byte[] bytes = Files.readAllBytes(Paths.get("/home/dkumar/RESEARCH/mnist_train.bin"));
        ByteBuffer buffer = ByteBuffer.wrap(bytes);
        buffer.order(ByteOrder.BIG_ENDIAN);

        int count = buffer.getInt();
        int rows = buffer.getInt();
        int cols = buffer.getInt();
        for (int m = 0; m < count; m++) {
            double[][] data = new double[rows][cols];
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++)
                    data[i][j] = buffer.getDouble();
            matrices.add(Mat.of(data));
        }
        matrices.forEach(elem -> elem.draw());
    }
}