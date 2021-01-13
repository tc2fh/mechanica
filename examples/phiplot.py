import math
import numpy as np

import matplotlib
import matplotlib.pyplot as plt

from time import time
from random import random

print ( matplotlib.__version__ )

class PhiPlot :


    def __init__(self, cell_type, origin=None, bincount=40):
        """
        cellType: cell type, or something that has an 'items' method
        origin: origin of positons
        bincount:
        """

        parts = cell_type.items()

        self.prev_phi = parts.spherical_positions()[:,2]

        # keep a handle on all the cells we've made.
        self.cell_type = cell_type

        # bins
        self.bins = np.linspace(0, np.pi, bincount)

    def phi_phidot(self, sph_positions):
        """
        returns two arrays,
        * the bin array, with the phi values
        * mean value of rate of change of phi for those bins
        """

        phi = sph_positions[:,2]

        phidot = phi - self.prev_phi

        self.prev_phi = phi

        digitized_phi = np.digitize(phi, self.bins)

        mean_phidot = np.array([phidot[digitized_phi == i].mean() for i range(len(self.bins))])

        return (self.bins, mean_phidot)


    def update(self, e):

        print("update(", self, ", ", e, ")")

        sph_positions = self.cell_type.items().spherical_positions()

        phi, phidot = self.phi_phidot(sph_positions)

        plt.pause(0.001)
        plt.clf()

        plt.plot(phi, phidot)
        plt.xlabel('$\phi$')
        plt.ylabel('${d \phi}/{dt}$')

        plt.show(block=False)