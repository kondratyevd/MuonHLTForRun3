import tensorflow
from tensorflow.python.keras.models import Model
from tensorflow.python.keras.layers import Dense, Input
from tensorflow.python.keras.layers import Dropout
from tensorflow.python.keras.layers import BatchNormalization


def get_model(label, input_dim, output_dim):
    inputs = Input(shape=(input_dim,), name=label+'_input')
    x = Dense(512, name=label+'_layer_1', activation='tanh')(inputs)
    x = Dropout(0.2)(x)
    x = BatchNormalization()(x)
    x = Dense(512, name=label+'_layer_2', activation='tanh')(x)
    x = Dropout(0.2)(x)
    x = BatchNormalization()(x)
    #x = Dense(128, name=label+'_layer_3', activation='tanh')(x)
    #x = Dropout(0.2)(x)
    #x = BatchNormalization()(x)
    outputs = Dense(output_dim, name=label+'_output',  activation='sigmoid')(x)

    model = Model(inputs=inputs, outputs=outputs)
    model.compile(
        loss='binary_crossentropy',
        optimizer='adam',
        metrics=["accuracy"])
    model.summary()
    return model, inputs.name, outputs.name

"""
#june 17
def get_model(label, input_dim, output_dim):
    inputs = Input(shape=(input_dim,), name=label+'_input')
    x = Dense(128, name=label+'_layer_1', activation='tanh')(inputs)
    x = Dropout(0.2)(x)
    x = BatchNormalization()(x)
    x = Dense(64, name=label+'_layer_2', activation='tanh')(x)
    x = Dropout(0.2)(x)
    x = BatchNormalization()(x)
    x = Dense(32, name=label+'_layer_3', activation='tanh')(x)
    x = Dropout(0.2)(x)
    x = BatchNormalization()(x)
    outputs = Dense(output_dim, name=label+'_output',  activation='sigmoid')(x)

    model = Model(inputs=inputs, outputs=outputs)
    model.compile(
        loss='binary_crossentropy',
        optimizer='adam',
        metrics=["accuracy"])
    model.summary()
    return model, inputs.name, outputs.name
"""
