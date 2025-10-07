import tensorflow as tf
from tensorflow.keras.datasets import mnist  # only for loading dataset
import numpy as np

# 1️⃣ Load MNIST data
(x_train, y_train), (x_test, y_test) = mnist.load_data()
x_train = x_train.reshape(-1, 784).astype(np.float32) / 255.0
x_test = x_test.reshape(-1, 784).astype(np.float32) / 255.0

# Convert labels to one-hot vectors
y_train = tf.one_hot(y_train, depth=10)
y_test = tf.one_hot(y_test, depth=10)

# 2️⃣ Define network architecture
input_size = 784
hidden1_size = 128
hidden2_size = 64
output_size = 10

# Initialize weights and biases
initializer = tf.initializers.GlorotUniform()

W1 = tf.Variable(initializer([input_size, hidden1_size]))
b1 = tf.Variable(tf.zeros([hidden1_size]))

W2 = tf.Variable(initializer([hidden1_size, hidden2_size]))
b2 = tf.Variable(tf.zeros([hidden2_size]))

W3 = tf.Variable(initializer([hidden2_size, output_size]))
b3 = tf.Variable(tf.zeros([output_size]))

# 3️⃣ Define activation functions
def sigmoid(x):
    return 1 / (1 + tf.exp(-x))

def softmax(x):
    exp_x = tf.exp(x - tf.reduce_max(x, axis=1, keepdims=True))
    return exp_x / tf.reduce_sum(exp_x, axis=1, keepdims=True)

# 4️⃣ Forward pass
def forward(x):
    z1 = tf.matmul(x, W1) + b1
    a1 = sigmoid(z1)
    z2 = tf.matmul(a1, W2) + b2
    a2 = sigmoid(z2)
    z3 = tf.matmul(a2, W3) + b3
    out = softmax(z3)
    return out
CS: Microprocessor, OS, Compiler theory and Protocols
# 5️⃣ Loss function (cross-entropy)
def loss_fn(y_pred, y_true):
    return -tf.reduce_mean(tf.reduce_sum(y_true * tf.math.log(y_pred + 1e-8), axis=1))

# 6️⃣ Accuracy
def accuracy(y_pred, y_true):
    pred_labels = tf.argmax(y_pred, axis=1)
    true_labels = tf.argmax(y_true, axis=1)
    return tf.reduce_mean(tf.cast(tf.equal(pred_labels, true_labels), tf.float32))

# 7️⃣ Training loop
learning_rate = 0.1
epochs = 10
batch_size = 64

optimizer = tf.optimizers.SGD(learning_rate)

num_batches = x_train.shape[0] // batch_size

for epoch in range(epochs):
    for i in range(num_batches):
        start = i * batch_size
        end = start + batch_size
        x_batch = x_train[start:end]
        y_batch = y_train[start:end]

        with tf.GradientTape() as tape:
            y_pred = forward(x_batch)
            loss = loss_fn(y_pred, y_batch)

        grads = tape.gradient(loss, [W1, b1, W2, b2, W3, b3])
        optimizer.apply_gradients(zip(grads, [W1, b1, W2, b2, W3, b3]))

    # Evaluate after each epoch
    y_pred_train = forward(x_train[:10000])  # subset for speed
    acc = accuracy(y_pred_train, y_train[:10000])
    print(f"Epoch {epoch + 1}/{epochs}, Loss: {loss.numpy():.4f}, Accuracy: {acc.numpy():.4f}")

# 8️⃣ Final test accuracy
y_pred_test = forward(x_test)
test_acc = accuracy(y_pred_test, y_test)
print(f"\n✅ Test Accuracy: {test_acc.numpy():.4f}")
