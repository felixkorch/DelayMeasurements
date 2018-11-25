<template>
    <div class='container' id="main_container">
        <div class='row'>
            <div class='col-lg-6 col-md-6 col-sm-12 col-xs-12'>
                <Table :passedData=siteData />
            </div>
            <div class='col-lg-6 col-md-6 col-sm-12 col-xs-12'>
                <SiteList :passedSites=sitesList v-on:siteSelected="siteClicked" v-on:siteAdded="siteAdded" />
            </div>
        </div>
        <div class="row">
            <LineChart :data=test />
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
        siteData: Object
    },
    data() {
        return {
            selectedSiteName: "",
            test: {
                labels: [new Date(100).toDateString(), 'b', 'c', 'd'],
                datasets: [ {
                    data: [20, 30, 40, 50],
                    label: 'Measurements',
                    backgroundColor: '#f87979'
                } ]
            }
        }
    },
    methods: {

        siteClicked: function(siteName) {
            this.selectedSiteName = siteName;
            this.fetchData(false);
        },

        siteAdded: function(siteName) {
            let ref = this;
            socket.emit('site_added', { site: siteName }, function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                console.log("successful!");
            });
        },

        fetchData: function(poll) {
            let ref = this;
            console.log("preparing to fetch data for " + this.selectedSiteName + "..");
            socket.emit('site_data', { "site_name": this.selectedSiteName, "poll": poll }, function(data) {
                if(!data.success) {
                    console.log("not successful");
                    return;
                }
                ref.siteData = data;
                console.log("successful!");
            });
        },

        init: function() {
            socket = io();
            let ref = this;

            socket.emit('site_list', function(data) {
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
                this.fetchData(true);
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
