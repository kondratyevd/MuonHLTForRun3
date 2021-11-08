import awkward as ak
import numpy as np
import scipy.stats

class MuCollection:
    def __init__(self, name='', ntuple_name='', branches=[], data={}):
        self.name = name
        self.ntuple_name = ntuple_name
        self.branches = branches
        self.data = data
        self.new_data = None
        self.new_data_dict = {}

    def __iadd__(self, other):
        for branch, branch_data in other.data.items():
            if branch in self.data.keys():
                if ak.count(self.data[branch], axis=None) == 0:
                    self.data[branch] = branch_data
                else:
                    self.data[branch] = ak.concatenate([self.data[branch], branch_data])
            else:
                self.data[branch] = branch_data
        return self

    def interpret_new_data(self, tree, cut):
        if cut is None:
            cut = tree[self.ntuple_name+'.pt'].array() > 0
        for branch in self.branches:
            self.new_data_dict[branch] = tree[self.ntuple_name+'.'+branch].array()[cut]
        # nVtx = tree['nVtx'].array()
        event = tree['eventNumber'].array()
        self.new_data_dict.update({
            # 'nVtx': ak.broadcast_arrays(nVtx, tree[self.ntuple_name+'.pt'].array()[cut])[0],
            'event': ak.broadcast_arrays(event, tree[self.ntuple_name+'.pt'].array()[cut])[0],
        })
        self.new_data = MuCollection(data=self.new_data_dict)

    def finalize(self):
        self += self.new_data


def match(first, second, **kwargs):
    if 'dR_cutoff' not in kwargs:
        raise Exception("Please specify dR cutoff for matching!")
    dR_cutoff = kwargs.pop('dR_cutoff', 0.3)
    return_match_properties = kwargs.pop('return_match_properties', False)
    etas = ak.cartesian(
        {'first': first.data['eta'], 'second': second.data['eta']},
        axis=1,
        nested=True
    )
    phis = ak.cartesian(
        {'first': first.data['phi'], 'second': second.data['phi']},
        axis=1,
        nested=True
    )
    dR, deta, dphi = delta_r(etas['first'], etas['second'], phis['first'], phis['second'])

    if return_match_properties:
        min_idx = ak.argmin(dR, axis=2)
        match_properties = {k:second.data[k][min_idx] for k in ['pt', 'eta', 'phi']}
        return ak.any(dR < dR_cutoff, axis=2), match_properties
    else:
        return ak.any(dR < dR_cutoff, axis=2)


def delta_r(eta1, eta2, phi1, phi2):
    deta = abs(eta1 - eta2)
    dphi = abs(np.mod(phi1 - phi2 + np.pi, 2*np.pi) - np.pi)
    dr = np.sqrt(deta**2 + dphi**2)
    return dr, deta, dphi


def mkdir(path):
    try:
        os.mkdir(path)
    except:
        pass