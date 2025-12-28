"""Tests for file readers."""

import pytest
import os


# Get the data directory path
DATA_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'data')


@pytest.fixture
def routing():
    """Import routing module."""
    import sys
    # Add the routing package to the path
    sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
    import routing
    return routing


class TestSolomonReader:
    """Tests for Solomon format reader."""

    def test_load_solomon_25(self, routing):
        """Test loading a 25-client Solomon instance."""
        filepath = os.path.join(DATA_DIR, 'VRPTWTD/Solomon/Instances/25/r101.txt')
        if not os.path.exists(filepath):
            pytest.skip(f"Test file not found: {filepath}")

        problem = routing.load_solomon(filepath)

        # Check problem structure
        assert problem is not None
        clients = problem.get_clients()
        assert len(clients) == 25

        vehicles = problem.get_vehicles()
        assert len(vehicles) > 0

    def test_load_solomon_50(self, routing):
        """Test loading a 50-client Solomon instance."""
        filepath = os.path.join(DATA_DIR, 'VRPTWTD/Solomon/Instances/50/r101.txt')
        if not os.path.exists(filepath):
            pytest.skip(f"Test file not found: {filepath}")

        problem = routing.load_solomon(filepath)

        clients = problem.get_clients()
        assert len(clients) == 50

    def test_solomon_reader_direct(self, routing):
        """Test using the reader module directly."""
        filepath = os.path.join(DATA_DIR, 'VRPTWTD/Solomon/Instances/25/r101.txt')
        if not os.path.exists(filepath):
            pytest.skip(f"Test file not found: {filepath}")

        from routing.readers.solomon import read_solomon
        problem = read_solomon(filepath)
        assert len(problem.get_clients()) == 25


class TestTSPLIBReader:
    """Tests for TSPLIB format reader."""

    def test_load_tsplib(self, routing):
        """Test loading a TSPLIB/CVRP instance."""
        filepath = os.path.join(DATA_DIR, 'CVRP/A/A-n55-k9.vrp')
        if not os.path.exists(filepath):
            pytest.skip(f"Test file not found: {filepath}")

        problem = routing.load_tsplib(filepath)

        # Check problem structure
        assert problem is not None
        clients = problem.get_clients()
        assert len(clients) == 54  # 55 nodes - 1 depot

        vehicles = problem.get_vehicles()
        assert len(vehicles) == 9  # k9 in filename

    def test_tsplib_reader_direct(self, routing):
        """Test using the reader module directly."""
        filepath = os.path.join(DATA_DIR, 'CVRP/A/A-n55-k9.vrp')
        if not os.path.exists(filepath):
            pytest.skip(f"Test file not found: {filepath}")

        from routing.readers.tsplib import read_tsplib
        problem = read_tsplib(filepath)
        assert len(problem.get_clients()) == 54
