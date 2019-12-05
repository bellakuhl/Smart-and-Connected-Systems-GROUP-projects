const path = require('path');

module.exports = {
  mode: "development",
  entry: {
    main: './src/index.tsx'
  },
  devtool: 'source-map',
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
      {
        enforce: "pre",
        test: /\.js$/,
        loader: "source-map-loader"
      }
    ],
  },
  resolve: {
    extensions: [ '.tsx', '.ts', '.js' ],
  },
  output: {
    filename: '[name].js',
    path: path.resolve(__dirname, './dist'),
  },
  externals: {
    "react": "React",
    "react-dom": "ReactDOM",
    "socket.io": "socket.io",
    "axios": "axios"
  },
  optimization: {
    splitChunks: {
      cacheGroups: {
        default: false,
        vendors: false,
        materialui: {
          chunks: 'all',
          test: /node_modules\/@material-ui/
        }
      }
    }
  }
};

