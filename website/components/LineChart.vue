<template>
  <div class="hello" ref="chartdiv">
  </div>
</template>

<script>
    export default {
        name: 'line-chart',
        props: {
            data: Object,
            visiblePoints: Number
        },
        data() {
            return {
                chart: null
            }
        },
        methods: {
            updateChart: function(newData, poll) {
                let nPoints = newData.times.length;
                if(!poll) {
                    let data = [];
                    if(nPoints >= this.visiblePoints) {
                        for (let i = nPoints - this.visiblePoints; i < nPoints; i++) {
                            data.push({ date: new Date(newData.times[i]), value: newData.durations[i] });
                        }
                    }else{
                        for (let i = 0; i < nPoints; i++) {
                            data.push({ date: new Date(newData.times[i]), value: newData.durations[i] });
                        }
                    }
                    this.chart.data = data;
                }else {
                    if(nPoints > this.visiblePoints)
                        this.chart.addData({ date: new Date(newData.times[nPoints - 1]), value: newData.durations[nPoints - 1] }, 1);
                    else
                        this.chart.addData({ date: new Date(newData.times[nPoints - 1]), value: newData.durations[nPoints - 1] });
                }
                //this.chart.xAxes.zoom({start:0, end:1}); Needed?
            }
        },
        ready() {
        },
        mounted() {
            let {am4core, am4charts, am4themes_animated, am4themes_dark} = this.$am4core();

            let chart = am4core.create(this.$refs.chartdiv, am4charts.XYChart);
            am4core.useTheme(am4themes_animated);
            let dateAxis = chart.xAxes.push(new am4charts.DateAxis());
            dateAxis.renderer.minGridDistance = 50;

            let valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
            valueAxis.tooltip.disabled = true;

            let series = chart.series.push(new am4charts.StepLineSeries());
            series.dataFields.dateX = "date";
            series.dataFields.valueY = "value";
            series.tooltipText = "{valueY.value}";
            series.strokeWidth = 3;

            chart.cursor = new am4charts.XYCursor();
            chart.cursor.xAxis = dateAxis;
            chart.cursor.fullWidthLineX = true;
            chart.cursor.lineX.strokeWidth = 0;
            chart.cursor.lineX.fill = chart.colors.getIndex(2);
            chart.cursor.lineX.fillOpacity = 0.1;

            //chart.events.on("ready", function () {
            //  dateAxis.zoom({start:0.79, end:1});
            //});

            this.chart = chart; 

        },

        beforeDestroy() {
            if (this.chart) {
                this.chart.dispose();
            }
        },

        created() {
        }
    }
</script>

<style scoped>
    .hello {
        width: 100%;
        height: 500px;
    }
</style>
