<template>
    <div class='container' id="main_container">
        <div class='row'>
            <div class='col-lg-6 col-md-6 col-sm-12 col-xs-12'>
                <Table :passedData=tableData />
            </div>
            <div class='col-lg-6 col-md-6 col-sm-12 col-xs-12'>
                <SiteList :passedSites=sitesList v-on:siteSelected="siteClicked" v-on:siteAdded="siteAdded" v-on:siteDeleted="siteDeleted" />
            </div>
        </div>
        <div class="row" style="margin-top:50px">
            <LineChart ref="lineChartRef" :visiblePoints=20 :chartData=chartData2 />
        </div>
    </div>
</template>

<script>
import io from 'socket.io-client';
//import VueSocketIO from 'vue-socket.io';

import SiteList from "~/components/SiteList";
import Table from "~/components/Table";
import LineChart from "~/components/LineChart";

var socket;

export default {
    name: 'App',
    components: {
        SiteList,
        Table,
        LineChart
    },
    props: {
        sitesList: Array,
        tableData: Object,
        chartData2: Array
    },
    data() {
        return {
            selectedSiteName: null
        }
    },
    methods: {

        siteClicked: function(siteName) {
            this.selectedSiteName = siteName;
            this.fetchData({ polling: false });
        },

        siteAdded: function(siteName) {
            let ref = this;
            socket.emit('siteAdded', { site: siteName }, function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                console.log("successful!");
            });
        },

        siteDeleted: function(siteName) {
            let ref = this;
            socket.emit('siteDeleted', { site: siteName }, function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                console.log("successful!");
            });
        },

        fetchData: function(options) {
            let ref = this;
            console.log("preparing to fetch data for " + this.selectedSiteName + "..");
            socket.emit('siteData', { "siteName": this.selectedSiteName, "options": options }, function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                ref.tableData = data;
                ref.chartData2 = data.chartData;
                ref.$refs.lineChartRef.updateChart(data.chartData, options.polling);
                console.log("successful!");
            });
        },

        init: function() {
            socket = io();
            let ref = this;

            socket.emit('siteList', function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                console.log("fetched list: " + data.list);
                ref.sitesList = data.list.map((element, index) => {
                    return { index: index, url: element, isSelected: false };
                });
            });

            setInterval(() => {
                if(this.selectedSiteName)
                    this.fetchData({ polling: true });
            }, 3000);
        }

    },
    mounted() {
        this.init();
    }
}

</script>

<style>
#app {
    font-family: 'Avenir', Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;
    margin-top: 60px;
}

#main_container {
    margin-top: 30px;
}
</style>
