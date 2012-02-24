include $(top_srcdir)/Makefile.common


if PYTHON
modpython_PYTHON = \
  __init__.py \
  raw_ofdm_params.py \
  raw_ofdm_sync.py \
  raw_ofdm_rx.py \
  raw_ofdm_mod.py \
  raw_qam.py
endif

EXTRA_DIST = run_tests.in

TESTS = run_tests

noinst_PYTHON = \
  qa_qam.py \
  qa_convenc.py \
  qa_intrlv.py \
  qa_fullstack.py \
  qa_rs.py
