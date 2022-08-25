module.exports = {
    root: true,
    env: {
      node: true
    },
    'extends': [
        "eslint:recommended",
        "plugin:react/recommended"
    ],
    rules: {
      'space-before-function-paren': [2, { 'anonymous': 'always', 'named': 'never' }],
      'no-console': 'off',
      'no-debugger': process.env.NODE_ENV === 'production' ? 'error' : 'off',
      "react/react-in-jsx-scope": "off",
      "react/prop-types": "off",
      "no-unused-vars": ["warn", { "vars": "all", "args": "after-used", "ignoreRestSiblings": false }],
    },
    parserOptions: {
      parser: 'babel-eslint'
    }
  }
  