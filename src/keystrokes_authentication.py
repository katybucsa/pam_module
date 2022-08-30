import sys
import joblib
import pandas as pd
import tensorflow as tf
from keras import Model, Sequential
from keras.layers import Dense, Dropout
from keras.saving.save import load_model

ENCODER_PATH = '/lib64/security/keystrokes_models/{0}/encoder.h5'
DECODER_PATH = '/lib64/security/keystrokes_models/{0}/decoder.h5'
SCALER_PATH = '/lib64/security/keystrokes_models/{0}/scaler.save'
USER_KEYSTROKES_PATH = '/lib64/security/keystrokes_models/{0}/{0}_keystrokes.csv'
THRESHOLDS_PATH = '/lib64/security/keystrokes_models/{0}/thresholds.txt'
PRED_PATH = '/lib64/security/keystrokes_models/{0}/pred_val.txt'

# create a model by subclassing Model class in tensorflow
class AutoEncoder(Model):
    """
    Parameters
    ----------
    output_units: int
      Number of output units

    code_size: int
      Number of units in bottleneck
    """

    def __init__(self, output_units, code_size=8):
        super().__init__()
        self.encoder = Sequential([
            Dense(64, activation='relu'),
            Dropout(0.1),
            Dense(32, activation='relu'),
            Dropout(0.1),
            Dense(16, activation='relu'),
            Dropout(0.1),
            Dense(code_size, activation='relu')
        ])
        self.decoder = Sequential([
            Dense(16, activation='relu'),
            Dropout(0.1),
            Dense(32, activation='relu'),
            Dropout(0.1),
            Dense(64, activation='relu'),
            Dropout(0.1),
            Dense(output_units, activation='sigmoid')
        ])

        # self.autoencoder = Model(output_units, self.decoder(self.encoder(output_units)),
        #                          name="autoencoder")

    def call(self, inputs):
        encoded = self.encoder(inputs)
        decoded = self.decoder(encoded)
        return decoded

def get_predictions(model, x_test_scaled, threshold):
    predictions = model.predict(x_test_scaled)
    # provides losses of individual instances
    errors = tf.keras.losses.msle(predictions, x_test_scaled)
    # 0 = anomaly, 1 = normal
    anomaly_mask = pd.Series(errors) > threshold
    preds = anomaly_mask.map(lambda x: 0.0 if x is True else 1.0)
    return preds

def authenticate_keystrokes(username):

    model = AutoEncoder(output_units=98)

    model.encoder = load_model(ENCODER_PATH.format(username))
    model.decoder = load_model(DECODER_PATH.format(username))


    scaler = joblib.load(SCALER_PATH.format(username))

    new_in = pd.read_csv(USER_KEYSTROKES_PATH.format(username), header=None).to_numpy()
    if len(new_in[0]) != 98:
        with open(PRED_PATH.format(username), 'w') as f:
            f.write("")
        return
    new_in_scaled = scaler.transform(new_in)


    with open(THRESHOLDS_PATH.format(username)) as f:
        firstline = f.readline().rstrip().split(",")
        threshold = float(firstline[0])
        threshold_2 = float(firstline[1])


    preds = get_predictions(model, new_in_scaled, threshold_2)
    with open(PRED_PATH.format(username), 'w') as f:
        f.write(f"{preds[0]}")

    return preds[0]

if __name__ == '__main__':
    authenticate_keystrokes(sys.argv[1])