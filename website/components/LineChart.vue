<template>
  <div class="hello" ref="chartdiv">
  </div>
</template>

<script>
    export default {
        name: 'line-chart',
        props: {
            visiblePoints: Number,
            chartData: Array
        },
        data() {
            return {
                chart: null,
                count: 0
            }
        },
        methods: {
            updateChart: function(chartData, poll) {
                if(poll) {
                    let lastIndex = chartData.length - 1;
                    let temp = { date: new Date(chartData[lastIndex].date), duration: chartData[lastIndex].duration };
                    this.chart.addData(temp, 1);
                }else {
                    chartData.forEach((el, i) => {
                        el.date = new Date(el.date);
                    });
                    if(this.count == 0) {
                        this.chart.data = chartData;
                        this.count++;
                        return;
                    }
                    chartData.forEach((el, i) => {
                        el.date = new Date(el.date);
                        this.chart.data[i].duration = el.duration;
                        this.chart.data[i].date = el.date;
                    });
                    this.chart.invalidateRawData();
                    //this.chart.xAxes.zoom({start:0, end:1});
                }
            }
        },
        ready() {
        },
        mounted() {
            let {am4core, am4charts, am4themes_animated, am4themes_dark} = this.$am4core();

            let chart = am4core.create(this.$refs.chartdiv, am4charts.XYChart);
            //am4core.useTheme(am4themes_animated);
            let dateAxis = chart.xAxes.push(new am4charts.DateAxis());
            dateAxis.renderer.minGridDistance = 60;

            let valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
            valueAxis.tooltip.disabled = true;

            let series = chart.series.push(new am4charts.StepLineSeries());
            series.dataFields.dateX = "date";
            series.dataFields.valueY = "duration";
            series.tooltipText = "{valueY.value}";
            series.strokeWidth = 3;

            chart.cursor = new am4charts.XYCursor();
            chart.cursor.xAxis = dateAxis;
            chart.cursor.fullWidthLineX = true;
            chart.cursor.lineX.strokeWidth = 0;
            chart.cursor.lineX.fill = chart.colors.getIndex(2);
            chart.cursor.lineX.fillOpacity = 0.1;

            dateAxis.baseInterval = {
		        "timeUnit": "second",
		        "count": 5
            }

            //chart.events.on("ready", function () {
            //  dateAxis.zoom({start:0.79, end:1});
            //});
            //chart.dataSource.url = "/api/sites/google.com";
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
