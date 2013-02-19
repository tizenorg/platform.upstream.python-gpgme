#!/usr/bin/env python

from distutils.core import setup, Extension

gpgme = Extension(
    'gpgme._gpgme',
    ['src/gpgme.c',
     'src/pygpgme-error.c',
     'src/pygpgme-data.c',
     'src/pygpgme-context.c',
     'src/pygpgme-key.c',
     'src/pygpgme-signature.c',
     'src/pygpgme-import.c',
     'src/pygpgme-keyiter.c',
     'src/pygpgme-constants.c',
     ],
    libraries=['gpgme'])

setup(name='pygpgme',
      version='0.1',
      author='James Henstridge',
      author_email='james@jamesh.id.au',
      description='A Python module for working with OpenPGP messages',
      long_description='''
          PyGPGME is a Python module that lets you sign, verify, encrypt
          and decrypt messages using the OpenPGP format.

          It is built on top of the GNU Privacy Guard and the GPGME
          library.''',
      license='LGPL',
      classifiers=[
          'Intended Audience :: Developers',
          'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
          'Operating System :: POSIX',
          'Programming Language :: C',
          'Programming Language :: Python',
          'Topic :: Security :: Cryptography',
          'Topic :: Software Development :: Libraries :: Python Modules'
      ],
      url='https://launchpad.net/products/pygpgme',
      ext_modules=[gpgme],
      packages=['gpgme', 'gpgme.tests'],
      package_data={'gpgme.tests': ['keys/*.pub', 'keys/*.sec']})
