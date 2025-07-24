import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Main {
    public static int argmax(Mat mat) {
        if (mat == null || mat.data.length == 0 || mat.data[0].length != 1)
            Error.fatal("Input must be a non-empty column matrix.");
        int maxIndex = 0;
        double maxValue = mat.data[0][0];
        for (int i = 1; i < mat.data.length; i++) {
            if (mat.data[i][0] > maxValue) {
                maxValue = mat.data[i][0];
                maxIndex = i;
            }
        }
        return maxIndex;
    }
    public static void main(final String[] args) throws Exception {
        List<Mat> dataX = new ArrayList<>();
        List<Mat> dataY = new ArrayList<>();

        byte[] bytes = Files.readAllBytes(Paths.get("/home/dkumar/RESEARCH/mnist_test.bin"));
        ByteBuffer buffer = ByteBuffer.wrap(bytes);
        buffer.order(ByteOrder.BIG_ENDIAN);

        int count = buffer.getInt();
        int rows = 28;
        int cols = 28;
        
        for (int m = 0; m < count; m++) {
            double[][] data = new double[rows][cols];
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++)
                    data[i][j] = buffer.getDouble();
            dataX.add(Mat.of(data));
        }
        for (int m = 0; m < count; m++) {
            double[][] data = new double[10][1];
            int val = buffer.getInt();
            data[val][0] = 1.0;
            dataY.add(Mat.of(data));
        }
        for (int a = 0; a < 100; a++) {
            dataX.get(a).draw();
            System.out.println(argmax(dataY.get(a)));
        }
    }
}