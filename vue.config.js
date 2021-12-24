const PUBLIC_GIT = false;
// vue.config.js
module.exports = {
    // 选项...
    publicPath: PUBLIC_GIT ? '/vh-webclient' : '/',
    // disable map
    productionSourceMap: false,
    outputDir: 'dist/',
    configureWebpack: config => {
        config.module.rules.push({
            test: /\.worker\.js$/,
            use: {
                loader: 'worker-loader',
                options: { inline: "fallback" }
            }
        })
    },
    parallel: false,
}