<template>
    <div class="container_div">
        <form @submit.prevent>
            <div class="input-group">
                <input
                    class="form-control"
                    @focus="inputFocused = true"
                    @blur="inputFocused = false"
                    v-bind:class="{ 'is-invalid': (newSiteError && newSiteError && newSite)}"
                    type="text" placeholder="http://example.com/"
                    v-model="newSite">
                <button
                    class="btn btn-primary"
                    :disabled="newSiteError"
                    type="submit"
                    @click="addSite"> Add Site </button>
            </div>
        </form>
        <ul class="list-group site-group">
            <!-- <transition-group name="splice" tag="div"> -->
                <li class="list-group-item site-panel"
                    v-for="item in sites"
                    @contextmenu.prevent="deleteSite(item)"
                    @click="itemClicked(item)"
                    v-bind:key="item"
                    v-bind:class="{ active: item.isSelected }"
                > {{ item.url }} </li>
            <!-- </transition-group> -->
        </ul>
    </div>
</template>

<script>
    export default {
        name: 'site-list',
        props: {
            passedSites: Array
        },
        data() {
            return {
                selectedSiteId: null,
                newSite: "",
                inputFocused: false
            }
        },

        methods: {
            deleteSite: function(item) {
                this.$emit('siteDeleted', item.url);
                this.sites.splice(item.index, 1);
                this.sites.forEach(element => {
                    if(element.index > item.index)
                        element.index--;
                });
            },

            addSite: function() {
                if(this.newSiteError)
                    return;
                this.sites.push({ url: this.newSite, index: this.sites.length, isSelected: false });
                this.$emit('siteAdded', this.newSite);
                this.newSite = "";
            },
            itemClicked: function(item) {
                if(this.selectedSiteId != null)
                    this.sites[this.selectedSiteId].isSelected = false;
                this.selectedSiteId = item.index;
                item.isSelected = true;
                this.$emit('siteSelected', item.url);
            },
            validURL: function(str) {
                var pattern = new RegExp('^(https?:\\/\\/)?'+ // protocol
                '((([a-z\\d]([a-z\\d-]*[a-z\\d])*)\\.)+[a-z]{2,}|'+ // domain name and extension
                '((\\d{1,3}\\.){3}\\d{1,3}))'+ // OR ip (v4) address
                '(\\:\\d+)?'+ // port
                '(\\/[-a-z\\d%@_.~+&:]*)*'+ // path
                '(\\?[;&a-z\\d%@_.,~+&:=-]*)?'+ // query string
                '(\\#[-a-z\\d_]*)?$','i'); // fragment locator
                return pattern.test(str);
            }
        },

        computed: {
            sites: function() {
                if(this.passedSites)
                    return this.passedSites;
                else
                    return [];
            },
            newSiteError: function() {
                if(this.validURL(this.newSite))
                    return false;
                return true;
            }
        }
    }
</script>

<style scoped>
.site-group {
    cursor: pointer;
    overflow-wrap: break-word;
    margin-top: 15px;
}

.splice-leave {
}

.splice-leave-active {
}

.splice-leave-to {
    opacity: 0;
    transform: translateX(100%);
}

.site-panel {
    overflow: hidden;
    border: 1px solid;
    text-align: center;
    display: inline-block;
    transition: all 0.3s;
}

</style>
