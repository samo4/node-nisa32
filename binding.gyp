{
  'targets': [
    {
      'target_name': 'nisa32',
      'sources': [
        'src/nisa32c.cpp'
      ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")'
      ],
      'conditions': [
        ['OS=="win"',
          {
            'include_dirs': [
              'C:\Program Files\IVI Foundation\VISA\Win64\Include'
            ],
            'link_settings': {
				'library_dirs': [
                  'C:\Program Files\IVI Foundation\VISA\Win64\Lib_x64\msc',
                ],
                'libraries': [
                  'visa64',
                ],
              },
            'msvs_settings': {
              'VCCLCompilerTool': {
                'ExceptionHandling': '2',
                'DisableSpecificWarnings': [ '4530', '4506' ],
              },
            },
          },
        ],
      ],
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ],
}