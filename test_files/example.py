#数据处理
import numpy as np
import codecs

#读取数据
with codecs.open(r'C:\Users\wis\Desktop\results\pos.txt','r') as f:
    reviews_pos = f.read()
with codecs.open(r'C:\Users\wis\Desktop\results\neg.txt','r','GBK') as f:
    reviews_neg = f.read()
labels = [1 for c in range(reviews_pos)] + [0 for c in range(reviews_pos)]
reviews = reviews_neg + reviews_pos

from string import punctuation

# 去除标点符号
all_text = ''.join([c for c in reviews if c not in punctuation])
# 每一个\n表示一条review
reviews = all_text.split('\n')

all_text = ' '.join(reviews)
# 获得所有单词
words = all_text.split()

#将文本转化为数字
from collections import Counter
def get_vocab_to_int(words):

    # 统计每个单词出现的次数
    counts = Counter(words)

    # 按出现次数，从多到少排序
    vocab = sorted(counts, key=counts.get, reverse=True)

    # 建立单词到数字的映射，也就是给单词贴上一个数字下标，在网络中用数字标签表示单词
    # 数字标签从 1 开始， 0 作特殊作用（下面会说）
    vocab_to_int = { word : i for i, word in enumerate(vocab, 1)}

    return vocab_to_int

def get_reviews_ints(vocab_to_int, reviews):
    # 将review转换为数字，也就是将review中每个单词，通过vocab_to_int转换为数字
    reviews_ints = []
    for each in reviews:
        reviews_ints.append( [ vocab_to_int[word] for word in each.split()] )

    return reviews_ints    

vocab_to_int = get_vocab_to_int(words)
reviews_ints = get_reviews_ints(vocab_to_int, reviews)
len(vocab_to_int)
labels = np.array([0 if label==0 else 1 for label in labels])

#清理垃圾数据
review_lens = Counter([len(x) for x in reviews_ints])
print('Zero-length reviews:{}'.format(review_lens[0]))
print("Maximum review length: {}".format(max(review_lens)))
non_zeros_idx = [ ii for ii, review in enumerate(reviews_ints) if len(review) != 0]
len(non_zeros_idx)
# 将长度为0的review从reviews_ints中清除
reviews_ints = [ reviews_ints[ii] for ii in non_zeros_idx]
labels = np.array( [ labels[ii] for ii in non_zeros_idx] )
# 字符序列长度
seq_len = 200
# 大小为 reviews的数量 * seq_len
features = np.zeros((len(reviews_ints), seq_len), dtype=int)
for i,review in enumerate(reviews_ints):
    features[i, -len(review):] = np.array(review)[:seq_len]

'''
建立训练集
'''
# 将训练集划分出来
split_frac = 0.8
# 将训练集划分出来
split_index = int(len(features)*split_frac)
train_x, val_x = features[:split_index], features[split_index:]
train_y, val_y = labels[:split_index], labels[split_index:]
print("\t\t\tFeature Shapes:")
print("Train set: \t\t{}".format(train_x.shape), 
      "\nValidation set: \t{}".format(val_x.shape),
      "\nTest set: \t\t{}".format(test_x.shape))

#设置基本参数
# LSTM 个数
lstm_size = 256
# LSTM 层数
lstm_layers = 1
batch_size = 512
learning_rate = 0.001

#定义输入输出
n_words = len(vocab_to_int)
# Create the graph object
graph = tf.Graph()
# Add nodes to the graph
with graph.as_default():
    # 输入变量，就是一条reviews，
    # 大小为[None, None]，第一个None表示batch_size,可以改为batch_size
    # 第二个None，表示输入review的长度，可以改成seq_len
    inputs_ = tf.placeholder(tf.int32, [None, None], name='inputs')
    # 输入标签
    labels_ = tf.placeholder(tf.int32, [None, None], name='labels')
    # dropout的概率，例如 0.8 表示80%不进行dropout
    keep_prob = tf.placeholder(tf.float32, name='keep_prob')
embed_size = 300
with graph.as_default():

    # 建立lstm层。这一层中，有 lstm_size 个 LSTM 单元
    lstm = tf.contrib.rnn.BasicLSTMCell(lstm_size)

    # 添加dropout
    drop = tf.contrib.rnn.DropoutWrapper(lstm, keep_prob)

    # 如果一层lsmt不够，多来几层
    cell = tf.contrib.rnn.MultiRNNCell([drop] * lstm_layers)

    # 对于每一条输入数据，都要有一个初始状态
    # 每次输入batch_size 个数据，因此有batch_size个初始状态
    initial_state = cell.zero_state(batch_size, tf.float32)

with graph.as_default():
    embedding = tf.Variable(tf.truncated_normal((n_words, embed_size), stddev=0.01))
    embed = tf.nn.embedding_lookup(embedding, inputs_)

with graph.as_default():
    outputs, final_state = tf.nn.dynamic_rnn(cell, embed, initial_state=initial_state)

with graph.as_default():
    # 我们只关心lstm最后的输出结果，因此outputs[:, -1]获取每条review最后一个单词的lstm层的输出
    # outputs[:, -1] 大小为 batch_size * lstm_size
    predictions = tf.contrib.layers.fully_connected(outputs[:, -1], 1, activation_fn=tf.sigmoid)
    cost = tf.losses.mean_squared_error(labels_, predictions)

    optimizer = tf.train.AdamOptimizer().minimize(cost)

with graph.as_default():
    correct_pred = tf.equal( tf.cast(tf.round(predictions), tf.int32), labels_ )
    accuracy = tf.reduce_mean(tf.cast(correct_pred, tf.float32))

#训练
epochs = 10

def get_batches(x, y, batch_size=100):
    n_batches = len(x) // batch_size
    x, y = x[:n_batches*batch_size], y[:n_batches*batch_size]

    for ii in range(0, len(x), batch_size):
        yield x[ii:ii+batch_size], y[ii:ii+batch_size]

# 持久化，保存训练的模型
with graph.as_default():
    saver = tf.train.Saver()

with tf.Session(graph=graph) as sess:
    tf.global_variables_initializer().run()
    iteration = 1

    for e in range(epochs):
        state = sess.run(initial_state)

        for ii, (x, y) in enumerate(get_batches(train_x, train_y, batch_size), 1):
            feed = {inputs_ : x,
                    labels_ : y[:,None],
                    keep_prob : 0.5,
                    initial_state : state}

            loss, state, _ = sess.run([cost, final_state, optimizer], feed_dict=feed)

            if iteration % 5 == 0:
                print('Epoch: {}/{}'.format(e, epochs),
                      'Iteration: {}'.format(iteration),
                      'Train loss: {}'.format(loss))

            if iteration % 25 == 0:
                val_acc = []
                val_state = sess.run(cell.zero_state(batch_size, tf.float32))

                for x, y in get_batches(val_x, val_y, batch_size):
                    feed = {inputs_ : x,
                            labels_ : y[:,None], 
                            keep_prob : 1,
                            initial_state : val_state}

                    batch_acc, val_state = sess.run([accuracy, final_state], feed_dict=feed)
                    val_acc.append(batch_acc)

                print('Val acc: {:.3f}'.format(np.mean(val_acc)))

            iteration += 1

    saver.save(sess, "checkpoints/sentiment.ckpt")

test_acc = []
with tf.Session(graph=graph) as sess:
    saver.restore(sess, tf.train.latest_checkpoint('checkpoints'))
    test_state = sess.run(cell.zero_state(batch_size, tf.float32))
    for ii, (x, y) in enumerate(get_batches(test_x, test_y, batch_size), 1):
        feed = {inputs_: x,
                labels_: y[:, None],
                keep_prob: 1,
                initial_state: test_state}
        batch_acc, test_state = sess.run([accuracy, final_state], feed_dict=feed)
        test_acc.append(batch_acc)
    print("Test accuracy: {:.3f}".format(np.mean(test_acc)))