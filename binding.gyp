{
  'variables': {
    'node_addon': '<!(node -p -e "require(\'path\').dirname(require.resolve(\'addon-layer\'))")',
  },
  'targets': [
    {
      'target_name': 'addon_test',
      'dependencies': [ '<(node_addon)/binding.gyp:addon-layer', ],
      'include_dirs': [ '<(node_addon)/src', ],
      'sources': [ 'src/test.c' ],
    },
  ],
}
