# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.

from mantidqt.widgets.workspacedisplay.table.io import TableWorkspaceDisplayDecoder, TableWorkspaceDisplayEncoder
from mantidqt.project.decoderfactory import DecoderFactory
from mantidqt.project.encoderfactory import EncoderFactory

DecoderFactory.register_decoder(TableWorkspaceDisplayDecoder)
EncoderFactory.register_encoder(TableWorkspaceDisplayEncoder)
