#!/usr/bin/env node

var glslifyDeps = require('glslify-deps');
var program = require('commander');

program
    .arguments('<file>')
    .option('-b, --base <directory>', 'Base directory for dependency finding')
    .action(function (file) {
        // chdir if needed
        if (program.directory !== undefined) {
            process.chdir(program.directory);
        }

        var depper = glslifyDeps();

        var i = 0;
        depper.on('file', function (file) {
            // The first file is always the entry point, it's not a dependency
            if (i > 0) {
                console.log(file);
            }

            i += 1;
        });

        depper.add(file, function (err, result) {
            if (err) {
                throw err;
            }
        });
    })
    .parse(process.argv);
