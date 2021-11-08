import pytest
import dask
from dask.distributed import Client
from dask.distributed import Scheduler, Worker
from dask_jobqueue import SLURMCluster
from datasets_7seeds import datasets, datasets_dict
from tools import build_dataset, preprocess, train
dask.config.set({"temporary-directory": "/depot/cms/hmm/dask-temp/"})
dask.config.set({'distributed.worker.timeouts.connect': '60s'})

__all__ = ['pytest', 'asyncio', 'dask',
           'Client', 'Scheduler', 'Worker',
           'SLURMCluster', 'dask_executor',
           'DimuonProcessor']

print('Dask version:', dask.__version__)
