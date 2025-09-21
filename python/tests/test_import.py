def test_import():
    import quasar

    assert isinstance(quasar.__version__, str)

