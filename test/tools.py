import glob
import time
import re
import json
from tqdm import tqdm
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import pickle

import uproot
import awkward as ak

import tensorflow as tf
import cmsml

import traceback

from utils import MuCollection, match, mkdir
from dnn_models import get_model

PT_CUT = 0
#PT_CUT = 24

MIN_EVENTS = 0 #85000
#MIN_EVENTS = 1000000
#MIN_EVENTS = 1750000
NFILES = 102

l2_branches = [
    'pt','eta', 'phi',
    'validHits',
    'tsos_IP_eta', 'tsos_IP_phi', 'tsos_IP_pt',
    'tsos_IP_pt_eta', 'tsos_IP_pt_phi',
    'err0_IP', 'err1_IP', 'err2_IP', 'err3_IP', 'err4_IP',
    'tsos_MuS_eta', 'tsos_MuS_phi', 'tsos_MuS_pt',
    'tsos_MuS_pt_eta', 'tsos_MuS_pt_phi',
    'err0_MuS', 'err1_MuS', 'err2_MuS', 'err3_MuS', 'err4_MuS',
    'tsos_IP_valid', 'tsos_MuS_valid'
]

l3_branches = ['pt', 'eta', 'phi']
gen_branches = ['pt', 'eta', 'phi', 'pdgId', 'status']


def build_dataset(dataset, intermediate_path, progress_bar=False):
    """
    'reference' muons are those for which we are building the dataset (e.g. L2s)
    'target' muons are those used to determine trigger decision (e.g. outside-in L3)
    """

    label = dataset['name']
    path = dataset['path']
    ref_name = dataset.pop('reference', 'L2muons')
    trg_name = dataset.pop('target', 'hltOImuons')
    gen_name = 'genParticles'

    coll_settings = {
        'ref': [ref_name, l2_branches],
        'trg': [trg_name, l3_branches],
        'gen': [gen_name, gen_branches]
    }

    # Initialize muon collections
    collections = {}
    for name, settings in coll_settings.items():
        collections[name] = MuCollection(
            name=name,
            ntuple_name=settings[0],
            branches=settings[1]
        )

    print('Loading '+label)

    abort = False
    try:
        p = intermediate_path+'/'+label+'.pickle'
        p = p.replace(',', '').replace(' ', '_')
        with open(p, 'rb') as handle:
            loaded = pickle.load(handle)
            abort = True
    except:
        pass
    if abort:
        return

    loop = tqdm(glob.glob(path)) if progress_bar else glob.glob(path)

    #if len(glob.glob(path))==NFILES:
    #    print('Skip because already completed: '+label)
    #    return

    i = 0
    for fname in loop:
        i+=1
        #if i==5:
        #    break

        # Open file
        try:
            tree = uproot.open(fname)['muonNtuples']['muonTree']['event']
        except:
            continue
        if len(tree['eventNumber'].array())==0:
            continue

        # Define cuts for each collection
        cuts = {
            'ref': (tree[ref_name+'.pt'].array()>0),
            'trg': None,  # take all
            'gen': (
                (abs(tree[gen_name+'.pdgId'].array())==13)&
                (tree[gen_name+'.status'].array()==1)&
                (tree[gen_name+'.pt'].array()>PT_CUT)&
                (abs(tree[gen_name+'.eta'].array())<2.4)
            )
        }

        # Convert branches from this file to jagged arrays
        for name, collection in collections.items():
            collection.interpret_new_data(tree, cuts[name])
     
        # Find L2s that have a matched L3 (for efficiency calculation)
        has_matched = match(
            collections['ref'].new_data,
            collections['trg'].new_data,
            dR_cutoff=0.3
        )
        # Find L2s that have a matched GEN
        has_matched_gen, closest_gen = match(
            collections['ref'].new_data,
            collections['gen'].new_data,
            dR_cutoff=0.3,
            return_match_properties=True
        )

        collections['ref'].new_data_dict['pass: '+label] = has_matched
        collections['ref'].new_data_dict['has_matched_gen'] = has_matched_gen
        collections['ref'].new_data_dict['gen_pt'] = closest_gen['pt']
        collections['ref'].new_data_dict['gen_eta'] = closest_gen['eta']
        collections['ref'].new_data_dict['gen_phi'] = closest_gen['phi']

        # Append new data to the dataset
        collections['ref'].finalize()

    save_path = intermediate_path+'/'+label+'.pickle'
    save_path = save_path.replace(',', '').replace(' ', '_')
    with open(save_path, 'wb') as handle:
        pickle.dump({label: collections['ref']}, handle, protocol=pickle.HIGHEST_PROTOCOL)

    print('Done: '+label)


def preprocess(paths, only_overlap_events=True):
    df = pd.DataFrame()

    if only_overlap_events:
        subset = ['event','subentry_']
    else:
        subset = ['label', 'event','subentry_']

    # Prepare index based on event number
    paths_filtered = []
    for path in tqdm(paths):
        try:
            with open(path, 'rb') as handle:
                src = pickle.load(handle)
                for label, collection in src.items():
                    df_ = ak.to_pandas(collection.data['event'])
                    df_.columns = ['event']
                    df_['subentry_'] = df_.index.get_level_values(1)
                    df_['label'] = label
                    #print(df_.shape[0])
                    if df_.shape[0]<MIN_EVENTS:
                        raise Exception
                    df = pd.concat([df, df_])
                    df = df.drop_duplicates(subset=subset)
            paths_filtered.append(path)
        except:
            #traceback.print_exc()
            continue
    df.set_index(subset, inplace=True)

    # Fill data from all sources
    for path in tqdm(paths_filtered):
        with open(path, 'rb') as handle:
            src = pickle.load(handle)
            for label, collection in src.items():
                df_ = ak.to_pandas(collection.data['event'])
                df_.columns = ['event']
                df_['label'] = label
                df_['subentry_'] = df_.index.get_level_values(1)
                if df_.shape[0]<MIN_EVENTS:
                    continue
                for branch, branch_data in collection.data.items():
                    if branch=='event':
                        continue
                    if branch in df.columns:
                        continue
                    df_[branch] = ak.to_pandas(branch_data)
                    df_reset = df_.reset_index().drop_duplicates(subset=subset).set_index(subset)
                    df.loc[df_reset.index, branch] = df_reset[branch]

    # If we want to only consider events that are present in all sources (needed for training)
    if only_overlap_events:
        to_drop = ['pt','eta','phi']
        df = df.drop_duplicates(subset=to_drop)
        df = df.dropna()
        for c in df.columns:
            if ('pass' in c):
                df[c] = df[c].astype(int)
    else:
        df = df.fillna(-999)

    df.reset_index(inplace=True)
    return df


def train(df, **kwargs):
    features = kwargs.pop('features', [])
    df_filter = kwargs.pop('filter', None)
    opt_label = kwargs.pop('label', 'test')
    output_path = kwargs.pop('output_path', './')
    save_model = kwargs.pop('save_model', False)
    save_loss_plot = kwargs.pop('save_loss_plot', False)
    save_metadata = kwargs.pop('save_metadata', False)
    epochs = kwargs.pop('epochs', 100)
    b_or_e = kwargs.pop('b_or_e', 'barrel')
    top_outputs = kwargs.pop('top_outputs', 1)
    mode = kwargs.pop('mode', 'multiple')

    output_path_full = output_path+'/'+opt_label
    mkdir(output_path)
    mkdir(output_path_full)

    truth_columns = []
    prediction_columns = []
    for c in df.columns:
        if ('pass' in c):
            truth_columns.append(c)

    truth_columns.sort()

    prediction_columns = [c.replace('pass', 'pred') for c in truth_columns]
    for c in prediction_columns:
        df[c] = -1

    if df_filter is not None:
        df = df[df_filter]

    if len(truth_columns)==0:
        return df
    
    # If only 1 strategy is considered,
    # then it is the best one automatically
    if len(truth_columns)==1:
        return df

    even = ((df.event % 2) == 0)
    df_train = df[even & ((df.event % 8) != 0)]
    df_val = df[even & ((df.event % 8) == 0)]
    # df_eval = df[~even]

    x_train = df_train[features]
    y_train = df_train[truth_columns].astype(int)
    x_val = df_val[features]
    y_val = df_val[truth_columns].astype(int)

    input_dim = len(features)
    output_dim = len(truth_columns)

    label = 'dnn_'+opt_label
    label = label.replace(' ', '_')
    label = label+'_'+b_or_e

    dnn, input_layer, output_layer = get_model(label, input_dim, output_dim)

    history = dnn.fit(
        x_train[features],
        y_train,
        epochs=epochs,
        batch_size=256,
        verbose=0,
        validation_data=(x_val[features], y_val),
        shuffle=True
    )


    if save_model:
        save_path = output_path_full+'/'+label+'.pb'
        cmsml.tensorflow.save_graph(save_path, dnn, variables_to_constants=True)
        cmsml.tensorflow.save_graph(save_path+'.txt', dnn, variables_to_constants=True)
        print('Saved model to '+save_path)

    if save_loss_plot:
        save_loss_plots(history, label, output_path_full)
        print('Saved loss plot to '+output_path_full)
        
    if save_metadata:
        metadata_pars = {}
        metadata_pars['input_layer'] = input_layer
        metadata_pars['output_layer'] = 'model/'+output_layer.replace(':0', '')
        metadata_pars['features'] = features
        metadata_pars['output_labels'] = truth_columns
        metadata_pars['dnn_name'] = label+'.pb'
        metadata_pars['data_path'] = 'RecoMuon/TrackerSeedGenerator/data/'
        metadata_pars['mode'] = mode
        if b_or_e == 'both':
            for boe in ['barrel', 'endcap']:
                metadata_pars['b_or_e'] = boe
                metadata_pars['out_path'] = output_path_full+'/metadata_'+boe+'.json'
                write_metadata(**metadata_pars)
        else:
            metadata_pars['b_or_e'] = b_or_e
            metadata_pars['out_path'] = output_path_full+'/metadata_'+b_or_e+'.json'
            write_metadata(**metadata_pars)
        print('Saved metadata json to '+metadata_pars['out_path'])

    prediction = pd.DataFrame(dnn.predict(df.loc[:, features]))
    df.loc[:, prediction_columns] = prediction.values
    df['best_guess_label'] = df[prediction_columns].idxmax(axis=1).str.replace('pred: ', 'pass: ')
    pred_label = 'DNN optimization: '+opt_label+' (top '+str(top_outputs)+')'
    
    if top_outputs==1:
        df.loc[:, pred_label] = df.lookup(df.index, df.best_guess_label)
    else:
        top_outputs = min(top_outputs, len(prediction_columns))
        arr = np.argsort(-df[prediction_columns].values, axis=1)
        df1 = pd.DataFrame(df[prediction_columns].columns[arr], index=df.index)
        best_guesses = []
        df[pred_label] = 0
        print(df1)
        for i in range(top_outputs):
            label_i = df1.iloc[:,i].str.replace('pred: ', 'pass: ')
            best_guesses.append(label_i)
            df.loc[:, pred_label] += df.lookup(df.index, label_i)
            df.loc[:, pred_label] = df.loc[:, pred_label].clip(0,1)

    pd.set_option('display.max_rows', None)
    n_strategies = len(prediction_columns)+1
    print('Strategies by average efficiency:')
    print(df.loc[~even, truth_columns+[pred_label]].mean().sort_values(ascending=False).head(n_strategies))
    print('---')
    print('Top strategies chosen by DNN:')
    print(df.loc[~even, 'best_guess_label'].value_counts().sort_values(ascending=False).head(n_strategies))

    return df


def save_loss_plots(history, label, out_path):
    fig = plt.figure()
    fig.clf()
    plt.rcParams.update({'font.size': 10})
    fig.set_size_inches(5, 4)
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title('model loss')
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'val'], loc='upper left')
    out = out_path+'/loss_'+label
    fig.savefig(out)
    print('Saved loss plot: '+out)


def write_metadata(**kwargs):
    features = kwargs.pop('features', [])
    b_or_e = kwargs.pop('b_or_e', 'barrel')
    data_path = kwargs.pop('data_path', 'RecoMuon/TrackerSeedGenerator/data/')
    dnn_name = kwargs.pop('dnn_name', 'dnn_5_seeds_0.pb')
    input_layer = kwargs.pop('input_layer', 'dnn_5_seeds_0_input')
    output_layer = kwargs.pop('output_layer', 'model/dnn_5_seeds_0_output/Sigmoid')
    output_labels = kwargs.pop('output_labels', [])
    out_path = kwargs.pop('out_path', './')

    mode = kwargs.pop('mode', 'multiple')

    n_features = len(features)
    
    metadata = {}
    metadata[b_or_e] = {}

    metadata[b_or_e]["dnnmodel_path"] = data_path+dnn_name
    metadata[b_or_e]["n_features"] = n_features
    metadata[b_or_e]["feature_names"] = features
    metadata[b_or_e]["output_labels"] = {}

    if mode == 'multiple': # multiple seeds per strategy
        for i, label in enumerate(output_labels):
            nHBd = int(re.findall(r'(\d+)HB\(d\)', label)[0])
            nHLIP = int(re.findall(r'(\d+)HL\(IP\)', label)[0])
            nHLMuS = int(re.findall(r'(\d+)HL\(MuS\)', label)[0])
            SF = int(re.findall(r'SF(\d+)', label)[0])
            metadata[b_or_e]["output_labels"]["label_"+str(i)] = {}
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["nHBd"] = nHBd
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["nHLIP"] = nHLIP
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["nHLMuS"] = nHLMuS
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["SF"] = SF
    elif mode == 'single': # 1-seed strategies
        for i, label in enumerate(output_labels):
            try:
                layerHBd = int(re.findall(r'HB\(d\) on layer (\d+)', label)[0])
            except:
                layerHBd = -1
            try:
                layerHLIP = int(re.findall(r'HL\(IP\) on layer (\d+)', label)[0])
            except:
                layerHLIP = -1
            try:
                layerHLMuS = int(re.findall(r'HL\(MuS\) on layer (\d+)', label)[0])
            except:
                layerHLMuS = -1
            #SF = int(re.findall(r'SF(\d+)', label)[0])
            metadata[b_or_e]["output_labels"]["label_"+str(i)] = {}
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["layerHBd"] = layerHBd
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["layerHLIP"] = layerHLIP
            metadata[b_or_e]["output_labels"]["label_"+str(i)]["layerHLMuS"] = layerHLMuS
            #metadata[b_or_e]["output_labels"]["label_"+str(i)]["SF"] = SF

    metadata[b_or_e]["input_layer"] = input_layer
    metadata[b_or_e]["output_layer"] = output_layer

    with open(out_path, 'w') as outfile:
        json.dump(metadata, outfile, indent=4)

