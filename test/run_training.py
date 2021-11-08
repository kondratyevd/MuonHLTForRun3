import os, sys
import time
import glob
import pickle
from tqdm import tqdm
from functools import partial

import pandas as pd
import awkward as ak
import dask
from dask.distributed import Client
import logging
logger = logging.getLogger("distributed.utils_perf")
logger.setLevel(logging.ERROR)

from datasets_7seeds import datasets, datasets_dict
#from datasets_layers import datasets, datasets_dict
from tools import build_dataset, preprocess, train, write_metadata

use_dask = False
local_cluster = True

step1 = False  # from ntuples to awkward arrays; save to disk
step2 = True  # from awkward arrays to Pandas, then train
save_model = True
save_loss_plot = True
save_metadata = True

OUTPUT_PATH = '/home/dkondra/muon-hlt-run3/CMSSW_11_2_0/src/HLTrigger/Configuration/python/MuonHLTForRun3/test/outputs/'
LABEL = 'training_jun27_2x512_noPtCut'
#B_OR_E = 'both'
#B_OR_E = 'barrel'
B_OR_E = 'endcap'

if B_OR_E == 'barrel':
    INTERMEDIATE_PATH = '/depot/cms/hmm/hlt-data/5_seeds_noPtCut/'
elif B_OR_E == 'endcap':
    INTERMEDIATE_PATH = '/depot/cms/hmm/hlt-data/7_seeds_noPtCut/'
else:
    INTERMEDIATE_PATH = '/depot/cms/hmm/hlt-data/layers/'


redo = [
]

features = [
    'validHits',
    'tsos_IP_eta', 'tsos_IP_phi', 'tsos_IP_pt',
    'tsos_IP_pt_eta', 'tsos_IP_pt_phi',
    'err0_IP', 'err1_IP', 'err2_IP', 'err3_IP', 'err4_IP',
    'tsos_MuS_eta', 'tsos_MuS_phi', 'tsos_MuS_pt',
    'tsos_MuS_pt_eta', 'tsos_MuS_pt_phi',
    'err0_MuS', 'err1_MuS', 'err2_MuS', 'err3_MuS', 'err4_MuS',
]


if __name__=='__main__':
    tick = time.time()

    rets = []

    if step1:
        #datasets = [d for d in datasets if "3HB(d), 3HL(IP)" in d["name"]]
        if use_dask:
            if local_cluster:
                n = min(os.cpu_count()-2, len(datasets))
                print('Using Dask with {0} local workers'.format(n))
                client = dask.distributed.Client(
                    processes=True,
                    n_workers=n,
                    threads_per_worker=1,
                    memory_limit='2GB'
                )
            else:
                client = Client(
                    '128.211.149.133:37573'
                )

        # Extract useful branches from all input sources
        build_dataset_ = partial(build_dataset, intermediate_path=INTERMEDIATE_PATH)
        if use_dask:
            print(client)
            rets = client.gather(client.map(build_dataset_, datasets))
        else:
            rets = []
            for ds in datasets:
                rets.append(build_dataset_(ds, progress_bar=True))
    
        tick1 = time.time()
        print('Loading took {0} s.'.format(tick1-tick))

    if step2:
        paths = glob.glob(INTERMEDIATE_PATH+'/*.pickle')
        #paths = [p for p in paths if "1HB(d)" in p]

        # Convert data into a Pandas dataframe
        # containing all trigger decisions for each L2
        df = preprocess(paths, only_overlap_events=True)
        print(df)
        print(df.columns)
        tick2 = time.time()

        # Train the DNN
        pars = {
            'features': features,
            'filter': df.has_matched_gen,
            'label': LABEL,
            'output_path': OUTPUT_PATH,
            'epochs': 100, #100
            'b_or_e': B_OR_E,
            'save_model': save_model,
            'save_loss_plot': save_loss_plot,
            'save_metadata': save_metadata,
            #'top_outputs': 5,
            'mode': 'multiple',
            #'mode': 'single',
        }
        df = train(df, **pars)

    tock = time.time()
    print('Completed in {0} s.'.format(tock-tick))









